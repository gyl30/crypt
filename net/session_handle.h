#ifndef LEAF_NET_SESSION_HANDLE_H
#define LEAF_NET_SESSION_HANDLE_H

#include "net/http_session.h"
#include "net/websocket_handle.h"

namespace leaf
{

struct session_handle
{
    // clang-format off
    std::function<leaf::websocket_handle::ptr(leaf::websocket_session::ptr &, const std::string &, const std::string &)> ws_handle;
    std::function<void(const leaf::http_session::ptr &, const leaf::http_session::http_request_ptr &)> http_handle;
    // clang-format on
};

}    // namespace leaf

#endif
