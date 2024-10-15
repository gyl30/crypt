#ifndef LEAF_BLAKE2B_H
#define LEAF_BLAKE2B_H

#include <vector>
#include <string>

namespace leaf
{
class blake2b
{
   public:
    blake2b();
    ~blake2b();

   public:
    std::string hex();
    std::vector<uint8_t> bytes();
    void update(const void* buffer, uint32_t buffer_len);
    void final();

   private:
    class blake2b_impl;
    blake2b_impl* impl = nullptr;
};
}    // namespace leaf

#endif