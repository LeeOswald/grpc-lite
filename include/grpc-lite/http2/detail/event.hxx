#pragma once

#include <grpc-lite/http2/detail/headers.hxx>


#include <cstdint>
#include <optional>
#include <string_view>

namespace grpc_lite::http2::detail
{

struct event 
{
    using header_t = std::optional<class header>;

    enum class type_t : std::uint8_t
    {
        noop = 0,
        stream_close,
        stream_data,
        stream_end,
        stream_header,
    };

    std::string_view data;
    std::int32_t stream_id = -1;
    type_t type = type_t::noop;

    header_t header = std::nullopt;
};

} // namespace grpc_lite::http2::detail {}
