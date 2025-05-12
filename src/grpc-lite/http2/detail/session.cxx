#include <grpc-lite/http2/detail/session.hxx>

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>


namespace grpc_lite::http2::detail
{

session::session() 
    : m_data()
    , m_events()
    , m_session(nullptr) 
{
    // initialize HTTP/2 session
    ::nghttp2_session_callbacks* callbacks;
    ::nghttp2_session_callbacks_new(&callbacks);

    ::nghttp2_session_callbacks_set_on_header_callback(callbacks, header_cb);
    ::nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, data_recv_cb);
    ::nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, frame_recv_cb);
    ::nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, stream_close_cb);

    ::nghttp2_session_server_new(&m_session, callbacks, this);

    ::nghttp2_session_callbacks_del(callbacks);

    // send HTTP/2 client connection header
    std::array<nghttp2_settings_entry, 1> iv = 
    {
        // FIXME: make concurrent streams configurable
        { NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 10 },
    };

    auto r = ::nghttp2_submit_settings(m_session, NGHTTP2_FLAG_NONE, iv.data(), iv.size());
    if (r != 0) 
    {
        throw std::runtime_error(::nghttp2_strerror(r));
    }
}

session::~session() 
{
    ::nghttp2_session_del(m_session);
}

void session::data(std::int32_t stream_id, std::string&& data)
{
    m_data = std::move(data);
    nghttp2_data_provider2 provider
    {
        .source =
        {
            .ptr = &m_data,
        },
        .read_callback = read_cb,
    };

    if (auto r = ::nghttp2_submit_data2(m_session, NGHTTP2_FLAG_NONE, stream_id, &provider); r != 0) 
    {
        throw std::runtime_error(std::string("Failed to submit data: ") + ::nghttp2_strerror(r));
    }
}

int session::data_recv_cb(
    nghttp2_session* session, 
    std::uint8_t flags, 
    std::int32_t stream_id, 
    const std::uint8_t* data, size_t len,
    void* vsess
) 
{
    auto sess = static_cast<class session*>(vsess);
    sess->emit({
        .data = {reinterpret_cast<const char*>(data), len},
        .stream_id = stream_id,
        .type = event::type_t::stream_data,
    });

    return 0;
}

void session::emit(event&& ev) noexcept 
{
    m_events.push_back(ev);
}

int session::frame_recv_cb(nghttp2_session* session, const nghttp2_frame* frame, void* vsess) 
{
    auto sess = static_cast<class session*>(vsess);
    if (0 != (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)) 
    {
        sess->emit({
            .stream_id = frame->hd.stream_id,
            .type = event::type_t::stream_end,
        });
    }

    return 0;
}

int session::header_cb(
    nghttp2_session* session, 
    const nghttp2_frame* frame, 
    const uint8_t* name, 
    size_t namelen,
    const uint8_t* value, 
    size_t valuelen, 
    uint8_t flags, 
    void* vsess
) 
{
    auto sess = static_cast<class session*>(vsess);
    sess->emit({
        .stream_id = frame->hd.stream_id,
        .type = event::type_t::stream_header,
        .header =
            header{
                .name = {reinterpret_cast<const char*>(name), namelen},
                .value = {reinterpret_cast<const char*>(value), valuelen},
            },
    });

    return 0;
}

void session::headers(std::int32_t stream_id, detail::headers hdrs) const 
{
    std::vector<nghttp2_nv> nv;
    nv.reserve(hdrs.size());

    for (const auto &h : hdrs) 
    {
        if (h.value.empty()) 
        {
            continue;
        }

        nv.push_back({
            const_cast<std::uint8_t*>(reinterpret_cast<const std::uint8_t*>(h.name.data())),
            const_cast<std::uint8_t*>(reinterpret_cast<const std::uint8_t*>(h.value.data())),
            h.name.size(),
            h.value.size(),
            NGHTTP2_NV_FLAG_NO_COPY_NAME | NGHTTP2_NV_FLAG_NO_COPY_VALUE,
        });
    }

    if (auto r = ::nghttp2_submit_headers(
        m_session, 
        NGHTTP2_FLAG_NONE, 
        stream_id, 
        nullptr, 
        nv.data(), 
        nv.size(), 
        nullptr);
        r != 0
        ) 
    {
        throw std::runtime_error(std::string("Failed to submit headers: ") + ::nghttp2_strerror(r));
    }
}

std::string_view session::pending() 
{
    const uint8_t* bytes;
    auto n = ::nghttp2_session_mem_send2(m_session, &bytes);
    if (n < 0) 
    {
        throw std::runtime_error(std::string("Failed to retrieve pending session data: ") + ::nghttp2_strerror(n));
    }

    return { reinterpret_cast<const char*>(bytes), static_cast<size_t>(n) };
}

session::events_t session::read(std::string_view bytes) 
{
    if (auto n = ::nghttp2_session_mem_recv2(m_session, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()); n < 0) 
    {
        throw std::runtime_error(std::string("Failed to read session data: ") + ::nghttp2_strerror(n));
    }

    auto events = m_events;
    m_events.clear();

    return events;
}

nghttp2_ssize session::read_cb(
    nghttp2_session* session, 
    std::int32_t stream_id, 
    std::uint8_t* buf, 
    std::size_t length, 
    std::uint32_t* data_flags,
    nghttp2_data_source* source, 
    void* vsess
) 
{
    auto str = static_cast<std::string*>(source->ptr);

    if (length >= str->size()) 
    {
        length = str->size();
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }

    std::memcpy(buf, str->data(), length);
    str->erase(0, length);

    return length;
}

int session::stream_close_cb(
    nghttp2_session* session, 
    std::int32_t stream_id, 
    std::uint32_t error_code, 
    void* vsess
) 
{
    auto sess = static_cast<class session*>(vsess);
    sess->emit({
        .stream_id = stream_id,
        .type = event::type_t::stream_close,
    });

    return 0;
}

void session::trailers(std::int32_t stream_id, detail::headers hdrs) const 
{
    std::vector<nghttp2_nv> nv;
    nv.reserve(hdrs.size());

    for (const auto& h : hdrs) 
    {
        if (h.value.empty()) 
        {
            continue;
        }

        nv.push_back({
            const_cast<std::uint8_t*>(reinterpret_cast<const std::uint8_t*>(h.name.data())),
            const_cast<std::uint8_t*>(reinterpret_cast<const std::uint8_t*>(h.value.data())),
            h.name.size(),
            h.value.size(),
            NGHTTP2_NV_FLAG_NO_COPY_NAME | NGHTTP2_NV_FLAG_NO_COPY_VALUE,
        });
    }

    if (auto r = nghttp2_submit_trailer(m_session, stream_id, nv.data(), nv.size()); r != 0) 
    {
        throw std::runtime_error(std::string("Failed to submit trailers: ") + ::nghttp2_strerror(r));
    }
}

} // namespace grpc_lite::http2::detail {}
