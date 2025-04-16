#ifndef LEAF_FILE_UPLOAD_FILE_HANDLE_H
#define LEAF_FILE_UPLOAD_FILE_HANDLE_H

#include <queue>
#include "file/file.h"
#include "protocol/codec.h"
#include "protocol/message.h"
#include "crypt/blake2b.h"
#include "file/file_context.h"
#include "net/websocket_handle.h"

namespace leaf
{

class upload_file_handle : public websocket_handle
{
   public:
    explicit upload_file_handle(std::string id);
    ~upload_file_handle() override;

   public:
    void startup() override;
    void shutdown() override;
    void update(const leaf::websocket_session::ptr&) override {}
    std::string type() const override { return "upload"; }
    void on_message(const leaf::websocket_session::ptr& session,
                    const std::shared_ptr<std::vector<uint8_t>>& bytes) override;

   private:
    void on_login(const std::optional<leaf::login_token>& l);
    void on_keepalive(const std::optional<leaf::keepalive>& k);
    void on_upload_file_request(const std::optional<leaf::upload_file_request>& u);
    void on_file_data(const std::optional<leaf::file_data>& d);

   private:
    enum upload_state : uint8_t
    {
        wait_login,
        wait_upload_request,
        wait_file_data
    };
    std::string id_;
    std::string user_;
    std::string token_;
    upload_state state_;
    std::vector<uint8_t> key_;
    leaf::file_context::ptr file_;
    std::shared_ptr<leaf::blake2b> hash_;
    std::shared_ptr<leaf::writer> writer_;
    std::queue<std::vector<uint8_t>> msg_queue_;
};

}    // namespace leaf
#endif
