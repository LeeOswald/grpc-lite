#pragma once

#include <grpc-lite/context.hxx>
#include <grpc-lite/status.hxx>


#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace grpc_lite
{

namespace detail 
{

class Request;
class Response;

} // namespace detail {}


class GRPC_LITE_EXPORT ServerBase 
{
public:
    using Fn = std::function<std::pair<Status, std::string>(Context&, std::string_view, std::string_view)>;

    using Services = std::unordered_map<std::string_view, Fn>;

    template <typename S> 
    void add(S& s) 
    {
        Fn fn = std::bind_front(&S::call, &s);
        m_services.insert({ s.name(), fn });
    }

protected:
    detail::Response process(const detail::Request &req) const noexcept;

private:
    Services m_services;
};

} // namespace grpc_lite {}