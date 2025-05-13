#include <grpc-lite/asio/detail/connection.hxx>
#include <grpc-lite/asio/detail/server.hxx>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>


namespace grpc_lite::asio::detail
{

boost::asio::awaitable<void> server::loop(boost::asio::ip::tcp::socket sock) 
{
    GrpcLiteVerboseBlock("{}.server::loop()", fmt::ptr(this));

    detail::connection c(std::move(sock));
    while (c) 
    {
        for (const auto &req : co_await c.reqs()) 
        {
            if (!c) 
            {
                break;
            }

            auto resp = process(req);
            co_await c.write(std::move(resp));
        }
    }
}

boost::asio::awaitable<void> server::listen(std::string_view ip, std::uint16_t port) 
{
    GrpcLiteVerboseBlock("{}.server::listen()", fmt::ptr(this));

    auto executor = co_await boost::asio::this_coro::executor;

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    boost::asio::ip::tcp::acceptor acceptor(executor, endpoint);

    for (;;) 
    {
        auto sock = co_await acceptor.async_accept(boost::asio::make_strand(executor), boost::asio::use_awaitable);

        co_spawn(sock.get_executor(), loop(std::move(sock)), boost::asio::detached);
    }
}

void server::run(std::string_view ip, std::uint16_t port) 
{
    GrpcLiteVerboseBlock("{}.server::run()", fmt::ptr(this));

    boost::asio::io_context ctx;
    co_spawn(ctx, listen(std::move(ip), std::move(port)), boost::asio::detached);

    ctx.run();
}

} // namespace grpc_lite::asio::detail {}
