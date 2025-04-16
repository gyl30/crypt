#include <utility>
#include <filesystem>

#include "log/log.h"
#include "file/file.h"
#include "crypt/easy.h"
#include "protocol/codec.h"
#include "file/download_file_handle.h"

constexpr auto kBlockSize = 128 * 1024;
constexpr auto kHashBlockCount = 10;

namespace leaf
{
download_file_handle::download_file_handle(std::string id, leaf::websocket_session::ptr& session)
    : id_(std::move(id)), session_(session)
{
    LOG_INFO("create {}", id_);
}

download_file_handle::~download_file_handle() { LOG_INFO("destroy {}", id_); }

void download_file_handle::startup()
{
    LOG_INFO("startup {}", id_);
    // clang-format off
    session_->set_read_cb(std::bind(&download_file_handle::on_read, this, std::placeholders::_1, std::placeholders::_2));
    session_->set_write_cb(std::bind(&download_file_handle::on_write, this, std::placeholders::_1, std::placeholders::_2));
    session_->startup();
    // clang-format on
}

void download_file_handle::shutdown()
{
    session_->shutdown();
    LOG_INFO("shutdown {}", id_);
}

void download_file_handle::on_write(boost::beast::error_code ec, std::size_t /*transferred*/)
{
    if (ec)
    {
        shutdown();
        return;
    }

    if (status_ != leaf::download_file_handle::status::wait_file_data)
    {
        return;
    }
    assert(file_->active_block_count <= file_->block_count);
    // read block data
    std::vector<uint8_t> buffer(kBlockSize, '0');
    auto read_size = reader_->read(buffer.data(), buffer.size(), ec);
    if (ec && ec != boost::asio::error::eof)
    {
        reset_status();
        return;
    }
    leaf::file_data fd;
    fd.block_id = file_->active_block_count;
    fd.data.swap(buffer);
    file_->active_block_count++;
    file_->hash_block_count++;
    hash_->update(buffer.data(), read_size);
    // block count hash or eof hash
    if (file_->hash_block_count == kHashBlockCount || ec == boost::asio::error::eof)
    {
        hash_->final();
        fd.hash = hash_->hex();
        file_->hash_block_count = 0;
        hash_ = std::make_shared<leaf::blake2b>();
    }
    if (!fd.data.empty())
    {
        auto bytes = leaf::serialize_file_data(fd);
        session_->write(bytes);
    }
    // eof reset
    if (ec == boost::asio::error::eof)
    {
        return;
    }
}
void download_file_handle::reset_status()
{
    auto _ = reader_->close();
    // clang-format off
        file_.reset(); reader_.reset(); hash_.reset();
    // clang-format on
    status_ = leaf::download_file_handle::status::wait_download_file_request;
}
void download_file_handle::on_read(boost::beast::error_code ec, const std::vector<uint8_t>& bytes)
{
    if (ec)
    {
        shutdown();
        return;
    }
    if (bytes.empty())
    {
        return;
    }
    auto type = get_message_type(bytes);
    if (type == leaf::message_type::login)
    {
        on_login(leaf::deserialize_login_token(bytes));
    }
    if (type == leaf::message_type::keepalive)
    {
        on_keepalive(leaf::deserialize_keepalive_response(bytes));
    }
    if (type == leaf::message_type::download_file_request)
    {
        on_download_file_request(leaf::deserialize_download_file_request(bytes));
    }
    if (type == leaf::message_type::file_data)
    {
        on_file_data(leaf::deserialize_file_data(bytes));
    }
}

void download_file_handle::on_file_data(const std::optional<leaf::file_data>& fd)
{
    if (status_ != wait_file_data)
    {
        LOG_ERROR("{} on_file_data status error", id_);
        shutdown();
        return;
    }
    if (!fd.has_value())
    {
        LOG_ERROR("{} on_file_data status error", id_);
        shutdown();
        return;
    }
}
void download_file_handle::on_login(const std::optional<leaf::login_token>& l)
{
    if (status_ != wait_login)
    {
        LOG_ERROR("{} on_login status error", id_);
        shutdown();
        return;
    }
    if (!l.has_value())
    {
        return;
    }
    token_ = l->token;
    LOG_INFO("{} on_login token {}", id_, token_);
    leaf::error_message err;
    err.id = l->id;
    err.error = 0;
    session_->write(leaf::serialize_error_message(err));
}

void download_file_handle::on_download_file_request(const std::optional<leaf::download_file_request>& download)
{
    if (status_ != wait_download_file_request)
    {
        LOG_ERROR("{} download_file status error", id_);
        shutdown();
        return;
    }
    if (!download.has_value())
    {
        return;
    }
    const auto& msg = download.value();
    std::string filename = leaf::encode(msg.filename);
    std::string download_file_path = leaf::make_file_path(token_, filename);
    download_file_path = leaf::make_normal_filename(download_file_path);
    LOG_INFO("{} download_file file {} to {}", id_, msg.filename, download_file_path);
    boost::system::error_code exists_ec;
    bool exist = std::filesystem::exists(download_file_path, exists_ec);
    if (exists_ec)
    {
        LOG_ERROR("{} download_file file {} exist error {}", id_, msg.filename, exists_ec.message());
        return;
    }
    if (!exist)
    {
        LOG_ERROR("{} download_file file {} not exist", id_, download_file_path);
        return;
    }

    std::error_code size_ec;
    auto file_size = std::filesystem::file_size(download_file_path, size_ec);
    if (size_ec)
    {
        LOG_ERROR("{} download_file file {} size error {}", id_, msg.filename, size_ec.message());
        return;
    }

    assert(!file_ && !reader_ && !hash_);
    reader_ = std::make_shared<leaf::file_reader>(download_file_path);
    auto ec = reader_->open();
    if (ec)
    {
        LOG_ERROR("{} download_file file open error {}", id_, ec.message());
        return;
    }
    if (reader_ == nullptr)
    {
        return;
    }
    uint64_t block_count = file_size / kBlockSize;
    uint32_t padding_size = 0;
    if (file_size % kBlockSize != 0)
    {
        padding_size = kBlockSize - (file_size % kBlockSize);
        block_count++;
    }
    hash_ = std::make_shared<leaf::blake2b>();
    file_ = std::make_shared<leaf::file_context>();
    file_->file_path = download_file_path;
    file_->file_size = file_size;
    file_->block_size = kBlockSize;
    file_->block_count = block_count;
    file_->padding_size = padding_size;
    file_->active_block_count = 0;
    file_->filename = msg.filename;
    LOG_INFO("{} download_file file {} size {}", id_, file_->file_path, file_->file_size);
    leaf::download_file_response response;
    response.filename = file_->filename;
    response.id = download->id;
    response.padding_size = padding_size;
    response.block_count = block_count;
    status_ = leaf::download_file_handle::status::wait_file_data;
    session_->write(leaf::serialize_download_file_response(response));
}

void download_file_handle::on_keepalive(const std::optional<leaf::keepalive>& k)
{
    if (!k.has_value())
    {
        return;
    }
    auto kk = k.value();
    kk.server_timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    LOG_DEBUG("{} on_keepalive client {} server_timestamp {} client_timestamp {} token {}",
              id_,
              kk.client_id,
              kk.server_timestamp,
              kk.client_timestamp,
              token_);
    session_->write(leaf::serialize_keepalive(kk));
}
void download_file_handle::on_error_message(const std::optional<leaf::error_message>& e)
{
    if (!e.has_value())
    {
        return;
    }
    LOG_INFO("{} download_file error {}", id_, e->error);
}
}    // namespace leaf
