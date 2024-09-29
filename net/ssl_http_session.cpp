#include "log.h"
#include "ssl_http_session.h"
#include "ssl_websocket_session.h"

namespace leaf
{

ssl_http_session::ssl_http_session(std::string id,
                                   boost::beast::tcp_stream&& stream,
                                   boost::asio::ssl::context& ctx,
                                   boost::beast::flat_buffer&& buffer,
                                   leaf::http_handle::ptr handle)
    : id_(std::move(id)), handle_(std::move(handle)), buffer_(std::move(buffer)), stream_(std::move(stream), ctx)
{
    LOG_INFO("create {}", id_);
}

ssl_http_session::~ssl_http_session() { LOG_INFO("destroy {}", id_); }

void ssl_http_session::startup()
{
    boost::asio::dispatch(stream_.get_executor(),
                          boost::beast::bind_front_handler(&ssl_http_session::safe_startup, this));
}

void ssl_http_session::safe_startup()
{
    LOG_INFO("startup {}", id_);
    boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    stream_.async_handshake(boost::asio::ssl::stream_base::server,
                            buffer_.data(),
                            boost::beast::bind_front_handler(&ssl_http_session::on_handshake, this));
}

void ssl_http_session::shutdown()
{
    boost::asio::dispatch(stream_.get_executor(),
                          boost::beast::bind_front_handler(&ssl_http_session::safe_shutdown, this));
}
void ssl_http_session::safe_shutdown()
{
    LOG_INFO("shutdown {}", id_);
    boost::system::error_code ec;
    ec = stream_.next_layer().socket().close(ec);
}

void ssl_http_session::on_handshake(boost::beast::error_code ec, std::size_t bytes_used)
{
    if (ec)
    {
        return shutdown();
    }

    buffer_.consume(bytes_used);

    do_read();
}
void ssl_http_session::do_read()
{
    boost::asio::dispatch(stream_.get_executor(), boost::beast::bind_front_handler(&ssl_http_session::safe_read, this));
}

void ssl_http_session::safe_read()
{
    parser_.emplace();

    parser_->body_limit(10000);

    boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    boost::beast::http::async_read(
        stream_, buffer_, *parser_, boost::beast::bind_front_handler(&ssl_http_session::on_read, this));
}
void ssl_http_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        return shutdown();
    }

    if (boost::beast::websocket::is_upgrade(parser_->get()))
    {
        boost::beast::get_lowest_layer(stream_).expires_never();
        return std::make_shared<ssl_websocket_session>(std::move(stream_))->startup(parser_->release());
    }

    auto req_ptr = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>(parser_->release());
    handle_->handle(shared_from_this(), req_ptr);
}

void ssl_http_session::write(const http_response_ptr& ptr)
{
    boost::asio::dispatch(stream_.get_executor(),
                          boost::beast::bind_front_handler(&ssl_http_session::safe_write, this, ptr));
}
void ssl_http_session::safe_write(const http_response_ptr& ptr)
{
    boost::beast::http::message_generator msg(std::move(*ptr));
    boost::beast::async_write(stream_,
                              std::move(msg),    //
                              boost::beast::bind_front_handler(&ssl_http_session::on_write, this));
}

void ssl_http_session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {}

}    // namespace leaf