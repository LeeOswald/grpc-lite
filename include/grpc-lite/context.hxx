#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <string_view>
#include <unordered_map>


namespace grpc_lite
{

namespace detail 
{

class request;

} // namespace detail {}


class GRPC_LITE_EXPORT context 
{
public:
    using meta_t = std::unordered_map<std::string_view, std::string_view>;

    context() = default;
    
    context(context&&) = default;
    
    context(const context&) = delete;

    context(const detail::request &req) noexcept;

    meta_t::mapped_type meta(meta_t::key_type key) const noexcept;

private:
    meta_t m_meta;
};


} // namespace grpc_lite {}