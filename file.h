#ifndef LEAF_FILE_H
#define LEAF_FILE_H

#include <string>
#include <boost/system/error_code.hpp>

namespace leaf
{
class file_impl;
class writer
{
   public:
    virtual ~writer() = default;

   public:
    virtual boost::system::error_code open() = 0;
    virtual boost::system::error_code close() = 0;
    virtual std::size_t write(void const* buffer, std::size_t size, boost::system::error_code& ec) = 0;
    virtual std::size_t size() = 0;
};
class reader
{
   public:
    virtual ~reader() = default;

   public:
    virtual boost::system::error_code open() = 0;
    virtual boost::system::error_code close() = 0;
    virtual std::size_t read(void* buffer, std::size_t size, boost::system::error_code& ec) = 0;
    virtual std::size_t size() = 0;
};

class null_writer : public writer
{
   public:
    ~null_writer() override = default;

   public:
    boost::system::error_code open() override { return {}; }
    boost::system::error_code close() override { return {}; }
    std::size_t write(void const* /*buffer*/, std::size_t size, boost::system::error_code& /*ec*/) override
    {
        size_ += size;
        return size;
    }
    std::size_t size() override { return size_; }

   private:
    std::size_t size_ = 0;
};
class null_reader : public reader
{
   public:
    ~null_reader() override = default;

   public:
    boost::system::error_code open() override { return {}; }
    boost::system::error_code close() override { return {}; }
    std::size_t read(void* /*buffer*/, std::size_t size, boost::system::error_code& /*ec*/) override
    {
        size_ += size;
        return size;
    }
    std::size_t size() override { return size_; }

   private:
    std::size_t size_ = 0;
};

class file_writer : public writer
{
   public:
    explicit file_writer(std::string filename);
    ~file_writer() override;

   public:
    boost::system::error_code open() override;
    boost::system::error_code close() override;
    std::size_t write(void const* buffer, std::size_t size, boost::system::error_code& ec) override;
    std::size_t size() override;

   private:
    file_impl* impl_ = nullptr;
};

class file_reader : public reader
{
   public:
    explicit file_reader(std::string filename);
    ~file_reader() override;

   public:
    boost::system::error_code open() override;
    boost::system::error_code close() override;
    std::size_t read(void* buffer, std::size_t size, boost::system::error_code& ec) override;
    std::size_t size() override;

   private:
    file_impl* impl_ = nullptr;
};

}    // namespace leaf

#endif
