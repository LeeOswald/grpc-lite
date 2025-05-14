#pragma once

#include <grpc-lite/http2/detail/event.hxx>

#if GRPC_LITE_WINDOWS
using ssize_t = std::make_signed_t<std::size_t>;
#endif

#include <nghttp2/nghttp2.h>


namespace grpc_lite::http2::detail
{

class GRPC_LITE_EXPORT Session 
{
public:
    using Events = std::vector<Event>;

    Session();
    Session(const Session &) = delete;

    ~Session();

    void headers(std::int32_t stream_id, detail::Headers hdrs) const;
    void data(std::int32_t stream_id, std::string &&data);
    void trailers(std::int32_t stream_id, detail::Headers hdrs) const;

    Events read(std::string_view bytes);
    std::string_view pending();

private:
    static int data_recv_cb(
        nghttp2_session* session, 
        std::uint8_t flags,
        std::int32_t stream_id,
        const std::uint8_t* data,
        std::size_t len,
        void* vsess
    );

    static int frame_recv_cb(
        nghttp2_session* session, 
        const nghttp2_frame* frame, 
        void* vsess
    );

    static int header_cb(
        nghttp2_session* session, 
        const nghttp2_frame* frame, 
        const std::uint8_t* name,
        std::size_t namelen,
        const std::uint8_t* value,
        std::size_t valuelen,
        std::uint8_t flags,
        void* vsess
    );

    static nghttp2_ssize read_cb(
        nghttp2_session* session, 
        std::int32_t stream_id,
        std::uint8_t* buf,
        std::size_t length,
        std::uint32_t* data_flags,
        nghttp2_data_source* source, 
        void* vsess
    );

    static int stream_close_cb(
        nghttp2_session* session, 
        std::int32_t stream_id,
        std::uint32_t error_code,
        void* vsess
    );

    void emit(Event&& ev) noexcept;

    std::string m_data;
    Events m_events;
    nghttp2_session* m_session;
};


} // namespace grpc_lite::http2::detail {}
