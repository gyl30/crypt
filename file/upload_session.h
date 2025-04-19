#ifndef LEAF_FILE_UPLOAD_SESSION_H
#define LEAF_FILE_UPLOAD_SESSION_H

#include <deque>
#include "file/file.h"
#include "file/event.h"
#include "protocol/codec.h"
#include "crypt/blake2b.h"
#include "file/file_context.h"
#include "net/plain_websocket_client.h"

namespace leaf
{
class upload_session : public std::enable_shared_from_this<upload_session>
{
   public:
    upload_session(std::string id,
                   std::string token,
                   leaf::upload_progress_callback cb,
                   boost::asio::ip::tcp::endpoint ed,
                   boost::asio::io_context &io);

    ~upload_session();

   public:
    void startup();
    void shutdown();
    void update();
    void add_file(const leaf::file_context::ptr &file);

   private:
    void on_read(boost::beast::error_code ec, const std::vector<uint8_t> &bytes);
    void on_write(boost::beast::error_code ec, std::size_t transferred);
    void on_connect(boost::beast::error_code ec);
    void update_process_file();
    void upload_file_request();
    void keepalive();
    void padding_file_event();
    void on_keepalive_response(const std::optional<leaf::keepalive> &);
    void on_upload_file_response(const std::optional<leaf::upload_file_response> &);
    void on_error_message(const std::optional<leaf::error_message> &);
    void on_login_token(const std::optional<leaf::login_token> &l);
    void upload_file_data();
    void emit_event(const leaf::upload_event &e);
    void reset_state();

   private:
    enum state : uint8_t
    {
        init,
        logined,
        upload_request,
        file_data,
    };
    state state_ = init;
    bool login_ = false;
    std::string id_;
    uint32_t seq_ = 0;
    std::string token_;
    boost::asio::io_context &io_;
    leaf::file_context::ptr file_;
    boost::asio::ip::tcp::endpoint ed_;
    std::shared_ptr<leaf::reader> reader_;
    std::shared_ptr<leaf::blake2b> hash_;
    leaf::upload_progress_callback progress_cb_;
    std::deque<leaf::file_context::ptr> padding_files_;
    std::shared_ptr<leaf::plain_websocket_client> ws_client_;
};
}    // namespace leaf

#endif
