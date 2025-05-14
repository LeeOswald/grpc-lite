#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <string_view>
#include <unordered_map>


namespace grpc_lite
{

namespace detail 
{

class Request;

} // namespace detail {}


class GRPC_LITE_EXPORT Context 
{
public:
    using Meta = std::unordered_map<std::string_view, std::string_view>;

    Context() = default;
    
    Context(Context&&) = default;
    
    Context(const Context&) = delete;

    Context(const detail::Request &req) noexcept;

    Meta::mapped_type meta(Meta::key_type key) const noexcept;

private:
    Meta m_meta;
};


} // namespace grpc_lite {}