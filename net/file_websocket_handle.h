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
    std::string id_;
};

}    // namespace leaf
