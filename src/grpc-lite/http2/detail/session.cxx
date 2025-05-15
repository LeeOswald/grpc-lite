#include <grpc-lite/http2/detail/session.hxx>

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>


namespace grpc_lite::http2::detail
{

Session::Session() 
    : m_data()
    , m_events()
    , m_session(nullptr) 
{
    GrpcLiteVerboseBlock("{}.Session::Session()", fmt::ptr(this));

    // initialize HTTP/2 Session
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

Session::~Session() 
{
    GrpcLiteVerboseBlock("{}.Session::~Session()", fmt::ptr(this));

    ::nghttp2_session_del(m_session);
}

void Session::setData(std::int32_t stream_id, std::string&& data)
{
    GrpcLiteVerboseBlock("{}.Session::data(stream_id={})", fmt::ptr(this), stream_id);

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

int Session::data_recv_cb(
    nghttp2_session* Session, 
    std::uint8_t flags, 
    std::int32_t stream_id, 
    const std::uint8_t* data, 
    size_t len,
    void* vsess
) 
{
    GrpcLiteVerboseBlock("Session::data_recv_cb(stream_id={}, data=[{}] [{}])", 
        stream_id, 
        debug::binaryToHex(std::string_view(reinterpret_cast<const char*>(data), len)),
        debug::binaryToAscii(std::string_view(reinterpret_cast<const char*>(data), len))
    );

    auto sess = static_cast<class Session*>(vsess);
    sess->emit({
        .data = {reinterpret_cast<const char*>(data), len},
        .stream_id = stream_id,
        .type = Event::Type::stream_data,
    });

    return 0;
}

void Session::emit(Event&& ev) noexcept 
{
    GrpcLiteVerboseBlock("{}.Session::emit()", fmt::ptr(this));

    m_events.push_back(ev);
}

int Session::frame_recv_cb(nghttp2_session* Session, const nghttp2_frame* frame, void* vsess) 
{
    GrpcLiteVerboseBlock("Session::frame_recv_cb()");

    auto sess = static_cast<class Session*>(vsess);
    if (0 != (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)) 
    {
        sess->emit({
            .stream_id = frame->hd.stream_id,
            .type = Event::Type::stream_end,
        });
    }

    return 0;
}

int Session::header_cb(
    nghttp2_session* Session, 
    const nghttp2_frame* frame, 
    const uint8_t* name, 
    size_t namelen,
    const uint8_t* value, 
    size_t valuelen, 
    uint8_t flags, 
    void* vsess
) 
{
    GrpcLiteVerboseBlock("Session::header_cb()");

    auto sess = static_cast<class Session*>(vsess);
    sess->emit({
        .stream_id = frame->hd.stream_id,
        .type = Event::Type::stream_header,
        .header =
            Header{
                .name = {reinterpret_cast<const char*>(name), namelen},
                .value = {reinterpret_cast<const char*>(value), valuelen},
            },
    });

    return 0;
}

void Session::setHeaders(std::int32_t stream_id, detail::Headers hdrs) const 
{
    GrpcLiteVerboseBlock("{}.Session::headers(stream_id={})", fmt::ptr(this), stream_id);

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

std::string_view Session::pending() 
{
    GrpcLiteVerboseBlock("{}.Session::pending()", fmt::ptr(this));

    const uint8_t* bytes;
    auto n = ::nghttp2_session_mem_send2(m_session, &bytes);
    if (n < 0) 
    {
        throw std::runtime_error(std::string("Failed to retrieve pending session data: ") + ::nghttp2_strerror(n));
    }

    return { reinterpret_cast<const char*>(bytes), static_cast<size_t>(n) };
}

Session::Events Session::read(std::string_view bytes) 
{
    GrpcLiteVerboseBlock("{}.Session::read(len={})", fmt::ptr(this), bytes.length());

    if (auto n = ::nghttp2_session_mem_recv2(m_session, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()); n < 0) 
    {
        throw std::runtime_error(std::string("Failed to read session data: ") + ::nghttp2_strerror(n));
    }

    auto events = m_events;
    m_events.clear();

    return events;
}

nghttp2_ssize Session::read_cb(
    nghttp2_session* Session, 
    std::int32_t stream_id, 
    std::uint8_t* buf, 
    std::size_t length, 
    std::uint32_t* data_flags,
    nghttp2_data_source* source, 
    void* vsess
) 
{
    GrpcLiteVerboseBlock("Session::read_cb(stream_id={}, length={})", stream_id, length);

    auto str = static_cast<std::string*>(source->ptr);

    if (length >= str->size()) 
    {
        length = str->size();
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }

    std::memcpy(buf, str->data(), length);
    str->erase(0, length);

    GrpcLiteVerbose("rd -> [{}] [{}]",
        debug::binaryToHex(std::string_view(reinterpret_cast<const char*>(buf), length)),
        debug::binaryToAscii(std::string_view(reinterpret_cast<const char*>(buf), length))
    );

    return length;
}

int Session::stream_close_cb(
    nghttp2_session* Session, 
    std::int32_t stream_id, 
    std::uint32_t error_code, 
    void* vsess
) 
{
    GrpcLiteVerboseBlock("Session::read_cb(stream_id={}, error_code={})", stream_id, error_code);

    auto sess = static_cast<class Session*>(vsess);
    sess->emit({
        .stream_id = stream_id,
        .type = Event::Type::stream_close,
    });

    return 0;
}

void Session::setTrailers(std::int32_t stream_id, detail::Headers hdrs) const 
{
    GrpcLiteVerboseBlock("{}.Session::setTrailers(stream_id={})", fmt::ptr(this), stream_id);

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
