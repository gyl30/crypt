#ifndef LEAF_UPLOAD_SESSION_H
#define LEAF_UPLOAD_SESSION_H

#include <queue>
#include "codec.h"
#include "blake2b.h"
#include "file_context.h"
#include "base_session.h"

namespace leaf
{
class upload_session : public leaf::base_session, public std::enable_shared_from_this<upload_session>
{
   public:
    explicit upload_session(std::string id);
    ~upload_session();

   public:
    void startup() override;
    void shutdown() override;
    void update() override;

   public:
    void add_file(const leaf::file_context::ptr &file) override;
    void on_message(const leaf::codec_message &msg) override;
    void set_message_cb(std::function<void(const leaf::codec_message &)> cb) override;

   private:
    void open_file();
    void upload_file_request();
    void upload_file_response(const leaf::upload_file_response &);
    void upload_file_exist(const leaf::upload_file_exist &);
    void delete_file_response(const leaf::delete_file_response &);
    void block_data_request(const leaf::block_data_request &);
    void file_block_request(const leaf::file_block_request &);
    void block_data_finish(const leaf::block_data_finish &);
    void error_response(const leaf::error_response &);
    void write_message(const codec_message &msg);

   private:
    std::string id_;
    leaf::file_context::ptr file_;
    std::shared_ptr<leaf::reader> reader_;
    std::shared_ptr<leaf::blake2b> blake2b_;
    std::queue<leaf::file_context::ptr> padding_files_;
    std::function<void(const leaf::codec_message &)> cb_;
};
}    // namespace leaf

#endif