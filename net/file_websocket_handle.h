#include "codec.h"
#include "message.h"
#include "blake2b.h"
#include "file_context.h"
#include "websocket_handle.h"

namespace leaf
{

class file_websocket_handle : public websocket_handle
{
   public:
    explicit file_websocket_handle(std::string id);
    ~file_websocket_handle() override;

   public:
    void startup() override;
    void on_text_message(const leaf::websocket_session::ptr& session,
                         const std::shared_ptr<std::vector<uint8_t>>& msg) override;
    void on_binary_message(const leaf::websocket_session::ptr& session,
                           const std::shared_ptr<std::vector<uint8_t>>& msg) override;
    void shutdown() override;

   private:
    void on_create_file_request(const leaf::create_file_request& msg);
    void on_delete_file_request(const leaf::delete_file_request& msg);
    void on_file_block_request(const leaf::file_block_request& msg);
    void on_block_data_request(const leaf::block_data_request& msg);
    void on_create_file_response(const leaf::create_file_response& msg);
    void on_delete_file_response(const leaf::delete_file_response& msg);
    void on_file_block_response(const leaf::file_block_response& msg);
    void on_block_data_response(const leaf::block_data_response& msg);
    void on_error_response(const leaf::error_response& msg);
    void block_data_request();
    void block_data_finish();
    void block_data_finish1(uint64_t file_id, const std::string& filename, const std::string& hash);
    void create_file_exist(const leaf::create_file_request& msg);
    void commit_message(const leaf::codec_message& msg);

   private:
    std::string id_;
    leaf::codec_handle handle_;
    leaf::file_context::ptr file_;
    std::shared_ptr<leaf::blake2b> hash_;
    std::shared_ptr<leaf::writer> writer_;
};

}    // namespace leaf
