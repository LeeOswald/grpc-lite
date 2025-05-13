#pragma once

#include <grpc-lite/server_base.hxx>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string_view>


namespace grpc_lite::asio::detail
{

class GRPC_LITE_EXPORT server 
    : public ::grpc_lite::server_base 
{
public:
    server(const server&) = delete;
    server() = default;

    boost::asio::awaitable<void> listen(std::string_view ip, std::uint16_t port);
    void run(std::string_view ip, std::uint16_t port);

private:
    boost::asio::awaitable<void> loop(boost::asio::ip::tcp::socket sock);
};

} // namespace grpc_lite::asio::detail {}
