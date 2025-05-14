#include <grpc-lite/asio/detail/connection.hxx>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>


namespace grpc_lite::asio::detail
{

Connection::Connection(boost::asio::ip::tcp::socket&& sock) noexcept 
    : m_socket(std::move(sock)) 
{
    GrpcLiteVerboseBlock("{}.Connection::Connection()", fmt::ptr(this));
}

Connection::requests_t Connection::read(std::size_t n) 
{
    GrpcLiteVerboseBlock("{}.Connection::read(n={})", fmt::ptr(this), n);

    requests_t reqs;
    for (auto &ev : m_session.read({ m_buffer.data(), n })) 
    {
        if (ev.stream_id <= 0) 
        {
            continue;
        }

        if (ev.type == http2::detail::Event::Type::stream_close) 
        {
            m_streams.erase(ev.stream_id);
            continue;
        }

        streams_t::iterator it = m_streams.emplace(ev.stream_id, ev.stream_id).first;
        auto& req = it->second;

        switch (ev.type) 
        {
        case http2::detail::Event::Type::stream_data: 
        {
            req.read(ev.data);
            break;
        }

        case http2::detail::Event::Type::stream_end: 
        {
            reqs.push_front(std::move(req));
            m_streams.erase(ev.stream_id);
            break;
        }

        case http2::detail::Event::Type::stream_header: 
        {
            req.header(std::move(ev.header->name), std::move(ev.header->value));
            break;
        }

        default:
            break;
        }
    }

    return reqs;
}

boost::asio::awaitable<Connection::requests_t> Connection::reqs() noexcept 
{
    GrpcLiteVerboseBlock("{}.Connection::reqs()", fmt::ptr(this));

    auto [ec, n] = co_await m_socket.async_read_some(
        boost::asio::buffer(m_buffer.data(), m_buffer.capacity()), 
        boost::asio::as_tuple(boost::asio::use_awaitable)
    );

    if (ec) 
    {
        // TODO: handle errors
        m_eos = true;
        co_return requests_t{};
    }

    requests_t reqs;
    try 
    {
        reqs = read(n);
    }
    catch (std::exception&) 
    {
        // TODO: handle errors
        m_eos = true;
        co_return requests_t{};
    }

    co_await write();
    co_return reqs;
}

boost::asio::awaitable<void> Connection::write() 
{
    GrpcLiteVerboseBlock("{}.Connection::write()", fmt::ptr(this));

    for (auto chunk = m_session.pending(); chunk.size() > 0; chunk = m_session.pending()) 
    {
        co_await boost::asio::async_write(m_socket, boost::asio::buffer(chunk), boost::asio::use_awaitable);
    }
}

boost::asio::awaitable<void> Connection::write(::grpc_lite::detail::Response resp) noexcept 
{
    GrpcLiteVerboseBlock("{}.Connection::write(resp)", fmt::ptr(this));

    m_session.headers(
        resp.id(),
        {
            {":status", "200"},
            {"content-type", "application/grpc"},
        });

    m_session.data(resp.id(), resp.bytes());
    co_await write();

    const auto &status = resp.status();
    m_session.trailers(
        resp.id(),
        {
            {"grpc-status", status},
            {"grpc-status-details-bin", status.details()},
        });

    co_await write();
}

} // namespace grpc_lite::asio::detail {}
