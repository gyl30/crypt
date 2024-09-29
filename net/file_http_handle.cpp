#include "file_http_handle.h"

namespace leaf
{

void file_http_handle::handle(const leaf::http_session::ptr &session, const leaf::http_session::http_request_ptr &req)
{
    boost::beast::http::response<boost::beast::http::string_body> response;
    response.result(boost::beast::http::status::ok);
    response.set(boost::beast::http::field::content_type, "text/plain");
    response.body() = "Hello, world!";
    response.prepare_payload();
    response.keep_alive(req->keep_alive());
    auto msg = std::make_shared<boost::beast::http::message_generator>(std::move(response));
    session->write(msg);
}
}    // namespace leaf