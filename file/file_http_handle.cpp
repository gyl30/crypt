#include <boost/algorithm/string.hpp>

#include "log/log.h"
#include "crypt/passwd.h"
#include "protocol/message.h"
#include "file/file_session.h"
#include "file/file_http_handle.h"
#include "file/cotrol_file_handle.h"
#include "file/upload_file_handle.h"
#include "file/download_file_handle.h"
#include "file/file_session_manager.h"

namespace leaf
{

leaf::websocket_handle::ptr websocket_handle(const std::string &id, const std::string &target)
{
    LOG_INFO("{} websocket handle target {}", id, target);
    if (boost::ends_with(target, "upload"))
    {
        return std::make_shared<upload_file_handle>(id);
    }
    if (boost::ends_with(target, "download"))
    {
        return std::make_shared<download_file_handle>(id);
    }
    if (boost::ends_with(target, "cotrol"))
    {
        return std::make_shared<cotrol_file_handle>(id);
    }

    return nullptr;
}

void http_handle(const leaf::http_session::ptr &session, const leaf::http_session::http_request_ptr &req)
{
    auto target = req->target();
    if (!target.ends_with("login"))
    {
        session->shutdown();
        return;
    }
    std::string data = req->body();
    leaf::read_buffer r(data.data(), data.size());
    uint64_t padding = 0;
    r.read_uint64(&padding);

    uint16_t type = 0;
    r.read_uint16(&type);
    if (type != leaf::to_underlying(leaf::message_type::login_request))
    {
        session->shutdown();
        return;
    }

    auto login_req = leaf::message::deserialize_login_request(r);
    if (!login_req)
    {
        session->shutdown();
        return;
    }

    leaf::login_token l;
    l.block_size = 128 * 1024;
    std::string token = leaf::passwd_hash(login_req->username, login_req->password);
    l.token = token;

    auto s = std::make_shared<leaf::file_session>();
    leaf::fsm::instance().add_session(token, s);

    boost::beast::http::response<boost::beast::http::string_body> response;
    response.result(boost::beast::http::status::ok);
    response.set(boost::beast::http::field::content_type, "text/plain");
    response.body() = leaf::serialize_message(l);
    response.prepare_payload();
    response.keep_alive(false);
    auto msg = std::make_shared<boost::beast::http::message_generator>(std::move(response));
    session->write(msg);
}
}    // namespace leaf
