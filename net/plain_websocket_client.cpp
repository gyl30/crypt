#include <utility>
#include "log/log.h"
#include "net/socket.h"
#include "net/buffer.h"
#include "net/plain_websocket_client.h"

namespace leaf
{

plain_websocket_client::plain_websocket_client(std::string id,
                                               std::string target,
                                               boost::asio::ip::tcp::endpoint ed,
                                               boost::asio::io_context& io)
    : id_(std::move(id)), target_(std::move(target)), ed_(std::move(ed)), ws_(io)
{
    LOG_INFO("create {}", id_);
}

plain_websocket_client::~plain_websocket_client()
{
    //
    LOG_INFO("destroy {}", id_);
}

void plain_websocket_client::startup()
{
    auto self = shared_from_this();
    boost::asio::dispatch(ws_.get_executor(),
                          boost::beast::bind_front_handler(&plain_websocket_client::safe_startup, self));
}

void plain_websocket_client::safe_startup()
{
    ws_.binary(true);
    LOG_INFO("startup {}", id_);
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(3));
    boost::beast::get_lowest_layer(ws_).async_connect(
        ed_, boost::beast::bind_front_handler(&plain_websocket_client::on_connect, shared_from_this()));
}

void plain_websocket_client::reconnect()
{
    LOG_INFO("reconnect {}", id_);
    auto timer = std::make_shared<boost::asio::steady_timer>(ws_.get_executor());
    auto self = shared_from_this();
    timer->expires_after(std::chrono::seconds(3));
    timer->async_wait([self, this, timer](boost::system::error_code ec) { on_reconnect(ec); });
}

void plain_websocket_client::on_reconnect(boost::beast::error_code ec)
{
    if (ec)
    {
        LOG_ERROR("{} reconnect failed {}", id_, ec.message());
        return;
    }
    if (ws_.is_open())
    {
        boost::system::error_code ec;
        ec = ws_.next_layer().socket().close(ec);
    }

    ws_.binary(true);
    LOG_INFO("on_reconnect {}", id_);
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(3));
    boost::beast::get_lowest_layer(ws_).async_connect(
        ed_, boost::beast::bind_front_handler(&plain_websocket_client::on_connect, shared_from_this()));
}

void plain_websocket_client::on_connect(boost::beast::error_code ec)
{
    if (ec)
    {
        LOG_ERROR("{} connect to {} failed {}", id_, leaf::get_endpoint_address(ed_), ec.message());
        on_error(ec);
        return;
    }
    LOG_INFO("{} connect to {}", id_, leaf::get_endpoint_address(ed_));
    auto self = shared_from_this();
    std::string host = leaf::get_endpoint_address(ed_);
    boost::beast::get_lowest_layer(ws_).expires_never();

    auto user_agent = [](auto& req) { req.set(boost::beast::http::field::user_agent, "leaf/ws"); };

    ws_.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
    ws_.set_option(boost::beast::websocket::stream_base::decorator(user_agent));

    ws_.async_handshake(host, target_, boost::beast::bind_front_handler(&plain_websocket_client::on_handshake, self));
}

void plain_websocket_client::on_handshake(boost::beast::error_code ec)
{
    if (ec)
    {
        LOG_ERROR("{} handshake failed {}", id_, ec.message());
        on_error(ec);
        return;
    }
    connected_ = true;
    if (handshake_handler_)
    {
        handshake_handler_(ec);
    }
    do_read();
    do_write();
}

void plain_websocket_client::do_read()
{
    boost::asio::dispatch(ws_.get_executor(),
                          boost::beast::bind_front_handler(&plain_websocket_client::safe_read, this));
}
void plain_websocket_client::safe_read()
{
    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
    ws_.async_read(buffer_, boost::beast::bind_front_handler(&plain_websocket_client::on_read, this));
}
void plain_websocket_client::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
    {
        LOG_ERROR("{} read failed {}", id_, ec.message());
        on_error(ec);
        return;
    }

    LOG_TRACE("{} read message size {}", id_, bytes_transferred);

    auto msg = leaf::buffers_to_vector(buffer_.data());

    buffer_.consume(buffer_.size());

    on_message(msg);

    do_read();
}
void plain_websocket_client::on_message(const std::shared_ptr<std::vector<uint8_t>>& msg)
{
    if (message_handler_)
    {
        message_handler_(msg, {});
    }
}
void plain_websocket_client::on_error(boost::beast::error_code ec)
{
    if (shutdown_)
    {
        return;
    }
    if (ec != boost::asio::error::operation_aborted)
    {
        message_handler_(nullptr, ec);
    }
    reconnect();
}

void plain_websocket_client::shutdown()
{
    shutdown_ = true;
    boost::asio::dispatch(ws_.get_executor(),
                          boost::beast::bind_front_handler(&plain_websocket_client::safe_shutdown, shared_from_this()));
}

void plain_websocket_client::safe_shutdown()
{
    connected_ = false;
    LOG_INFO("shutdown {}", id_);
    boost::beast::error_code ec;
    ec = ws_.next_layer().socket().close(ec);
}

void plain_websocket_client::write(std::vector<uint8_t> msg)
{
    boost::asio::dispatch(ws_.get_executor(),
                          boost::beast::bind_front_handler(&plain_websocket_client::safe_write, this, msg));
}

void plain_websocket_client::safe_write(const std::vector<uint8_t>& msg)
{
    assert(!msg.empty());
    msg_queue_.push(msg);
    do_write();
}
void plain_websocket_client::do_write()
{
    if (msg_queue_.empty())
    {
        return;
    }
    if (writing_)
    {
        return;
    }
    if (!connected_)
    {
        return;
    }
    writing_ = true;
    auto& msg = msg_queue_.front();

    ws_.async_write(boost::asio::buffer(msg),
                    boost::beast::bind_front_handler(&plain_websocket_client::on_write, this));
}

void plain_websocket_client::on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
    {
        LOG_ERROR("{} write failed {}", id_, ec.message());
        on_error(ec);
        return;
    }
    LOG_TRACE("{} write success {} bytes", id_, bytes_transferred);
    writing_ = false;
    msg_queue_.pop();
    do_write();
}
}    // namespace leaf
