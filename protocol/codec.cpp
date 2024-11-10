#include <cassert>
#include "codec.h"
#include "reflect.hpp"
#include "net_buffer.h"

#include <type_traits>

namespace reflect
{
REFLECT_STRUCT(leaf::upload_file_request, (file_size)(hash)(filename));
REFLECT_STRUCT(leaf::block_data_finish, (file_id)(hash)(filename));
REFLECT_STRUCT(leaf::upload_file_exist, (hash)(filename));
REFLECT_STRUCT(leaf::download_file_response, (hash)(filename)(file_size)(file_id));
}    // namespace reflect

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
namespace leaf
{
uint16_t to_underlying(leaf::message_type type) { return ::to_underlying(type); }
}    // namespace leaf

static void write_padding(leaf::write_buffer &w)
{
    uint64_t xx = 0;       // NOLINT
    w.write_uint64(xx);    // NOLINT
}

namespace leaf
{
std::vector<uint8_t> serialize_upload_file_request(const upload_file_request &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::upload_file_request));
    std::string str = reflect::serialize_struct(msg);
    w.write_bytes(str.data(), str.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_upload_file_exist(const upload_file_exist &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::upload_file_exist));
    std::string str = reflect::serialize_struct(msg);
    w.write_bytes(str.data(), str.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}

std::vector<uint8_t> serialize_upload_file_response(const upload_file_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::upload_file_response));
    w.write_uint64(msg.file_id);
    w.write_bytes(msg.filename.data(), msg.filename.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_download_file_request(const download_file_request &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::download_file_request));
    w.write_bytes(msg.filename.data(), msg.filename.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_download_file_response(const download_file_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::download_file_response));
    std::string str = reflect::serialize_struct(msg);
    w.write_bytes(str.data(), str.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}

std::vector<uint8_t> serialize_delete_file_request(const delete_file_request &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::delete_file_request));
    w.write_bytes(msg.filename.data(), msg.filename.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_file_block_request(const file_block_request &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::file_block_request));
    w.write_uint64(msg.file_id);
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_delete_file_response(const delete_file_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::delete_file_response));
    w.write_bytes(msg.filename.data(), msg.filename.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_block_data_request(const block_data_request &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::block_data_request));
    w.write_uint64(msg.file_id);
    w.write_uint32(msg.block_id);
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}

std::vector<uint8_t> serialize_file_block_response(const file_block_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::file_block_response));
    w.write_uint64(msg.file_id);
    w.write_uint32(msg.block_size);
    w.write_uint32(msg.block_count);
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_block_data_response(const block_data_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::block_data_response));
    w.write_uint64(msg.file_id);
    w.write_uint32(msg.block_id);
    w.write_bytes(msg.data.data(), msg.data.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}
std::vector<uint8_t> serialize_block_data_finish(const block_data_finish &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::block_data_finish));
    std::string str = reflect::serialize_struct(msg);
    w.write_bytes(str.data(), str.size());
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}

std::vector<uint8_t> serialize_error_response(const error_response &msg)
{
    leaf::write_buffer w;
    write_padding(w);
    w.write_uint16(leaf::to_underlying(message_type::error));
    w.write_uint32(msg.error);
    std::vector<uint8_t> bytes;
    w.copy_to(&bytes);
    return bytes;
}

static std::optional<leaf::upload_file_request> decode_upload_file_request(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }

    std::string str;
    r.read_string(&str, r.size());
    if (str.empty())
    {
        return {};
    }
    leaf::upload_file_request req;
    if (!reflect::deserialize_struct(req, str))
    {
        return {};
    }
    return req;
}

static std::optional<leaf::upload_file_exist> decode_upload_file_exist(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    std::string str;
    r.read_string(&str, r.size());
    if (str.empty())
    {
        return {};
    }

    leaf::upload_file_exist create;
    if (!reflect::deserialize_struct(create, str))
    {
        return {};
    }
    return create;
}

static std::optional<leaf::delete_file_request> decode_delete_file_request(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    std::string filename;
    r.read_string(&filename, r.size());
    if (filename.empty())
    {
        return {};
    }
    leaf::delete_file_request req;
    req.filename = filename;
    return req;
}
static std::optional<leaf::upload_file_response> decode_upload_file_response(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    uint64_t file_id = 0;
    r.read_uint64(&file_id);
    std::string filename;
    r.read_string(&filename, r.size());
    leaf::upload_file_response create;
    create.filename = filename;
    create.file_id = file_id;
    return create;
}
static std::optional<leaf::file_block_request> decode_file_block_request(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    uint64_t file_id = 0;
    r.read_uint64(&file_id);
    leaf::file_block_request req;
    req.file_id = file_id;
    return req;
}

static std::optional<leaf::file_block_response> decode_file_block_response(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    uint64_t file_id = 0;
    r.read_uint64(&file_id);

    uint32_t block_size = 0;
    r.read_uint32(&block_size);

    uint32_t block_count = 0;
    r.read_uint32(&block_count);

    leaf::file_block_response res;
    res.file_id = file_id;
    res.block_count = block_count;
    res.block_size = block_size;
    return res;
}
static std::optional<leaf::block_data_request> decode_block_data_request(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    uint64_t file_id = 0;
    r.read_uint64(&file_id);
    uint32_t block_id = 0;
    r.read_uint32(&block_id);

    leaf::block_data_request req;
    req.file_id = file_id;
    req.block_id = block_id;
    return req;
}
static std::optional<leaf::block_data_response> decode_block_data_response(leaf::read_buffer &r)
{
    uint64_t file_id = 0;
    r.read_uint64(&file_id);
    uint32_t block_id = 0;
    r.read_uint32(&block_id);
    std::vector<uint8_t> block_data(r.size(), '0');
    r.read_bytes(block_data.data(), r.size());
    leaf::block_data_response block;
    block.file_id = file_id;
    block.block_id = block_id;
    block.data.swap(block_data);
    return block;
}
static std::optional<leaf::block_data_finish> decode_block_data_finish(leaf::read_buffer &r)
{
    if (r.size() > 2048)
    {
        return {};
    }
    std::string str;
    r.read_string(&str, r.size());
    if (str.empty())
    {
        return {};
    }
    leaf::block_data_finish finish;
    if (!reflect::deserialize_struct(finish, str))
    {
        return {};
    }
    return finish;
}
std::vector<uint8_t> serialize_message(const codec_message &msg)
{
    struct message_serializer
    {
#define XXX(xx) \
    std::vector<uint8_t> operator()(const leaf::xx &arg) const { return serialize_##xx(arg); }
        XXX(upload_file_request)
        XXX(delete_file_request)
        XXX(file_block_request)
        XXX(block_data_request)
        XXX(upload_file_response)
        XXX(delete_file_response)
        XXX(file_block_response)
        XXX(block_data_response)
        XXX(error_response)
        XXX(block_data_finish)
        XXX(upload_file_exist)
        XXX(download_file_request)
        XXX(download_file_response)
#undef XXX
    };
    return std::visit(message_serializer{}, msg);
}

std::optional<codec_message> deserialize_message(const uint8_t *data, uint64_t len)
{
    leaf::read_buffer r(data, len);
    uint64_t padding = 0;
    r.read_uint64(&padding);

    uint16_t type = 0;
    r.read_uint16(&type);

    switch (static_cast<leaf::message_type>(type))
    {
        case leaf::message_type::upload_file_request:
            return decode_upload_file_request(r);
        case leaf::message_type::delete_file_request:
            return decode_delete_file_request(r);
        case leaf::message_type::upload_file_response:
            return decode_upload_file_response(r);
        case leaf::message_type::file_block_request:
            return decode_file_block_request(r);
        case leaf::message_type::file_block_response:
            return decode_file_block_response(r);
        case leaf::message_type::block_data_request:
            return decode_block_data_request(r);
        case leaf::message_type::block_data_response:
            return decode_block_data_response(r);
        case leaf::message_type::block_data_finish:
            return decode_block_data_finish(r);
        case leaf::message_type::upload_file_exist:
            return decode_upload_file_exist(r);
        case leaf::message_type::download_file_request:
        case leaf::message_type::download_file_response:
        case leaf::message_type::delete_file_response:
        case leaf::message_type::error:
            return {};
    }
    return {};
}

}    // namespace leaf
