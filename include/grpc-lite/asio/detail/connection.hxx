#pragma once

#include <grpc-lite/http2/detail/session.hxx>
#include <grpc-lite/detail/request.hxx>
#include <grpc-lite/detail/response.hxx>


#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <forward_list>
#include <unordered_map>


namespace grpc_lite::asio::detail
{

class GRPC_LITE_EXPORT Connection 
{
public:
    using requests_t = std::forward_list<::grpc_lite::detail::Request>;
    using streams_t = std::unordered_map<int32_t, ::grpc_lite::detail::Request>;

    Connection(const Connection &) = delete;
    Connection(boost::asio::ip::tcp::socket&& sock) noexcept;

    operator bool() const noexcept 
    { 
        return !m_eos; 
    }

    boost::asio::awaitable<requests_t> reqs() noexcept;
    boost::asio::awaitable<void> write(::grpc_lite::detail::Response resp) noexcept;

private:
    template <std::size_t N> 
    class Buffer 
    {
    public:
        constexpr char *data() noexcept 
        { 
            return &_data[0]; 
        }

        constexpr std::size_t capacity() const noexcept 
        { 
            return N; 
        }

    private:
        char _data[N];
    };

    requests_t read(std::size_t n);
    boost::asio::awaitable<void> write();

    Buffer<1024> m_buffer; // FIXME: make size configurable
    bool m_eos = false;
    http2::detail::Session m_session;
    streams_t m_streams;
    boost::asio::ip::tcp::socket m_socket;
};


} // namespace grpc_lite::asio::detail {}
