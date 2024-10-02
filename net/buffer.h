#include <vector>
#include <boost/asio/buffer.hpp>

namespace leaf
{

std::shared_ptr<std::vector<uint8_t>> buffers_to_vector(const boost::asio::mutable_buffer& buffers);

}    // namespace leaf