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

class request;
class response;

} // namespace detail {}


class GRPC_LITE_EXPORT server_base 
{
public:
    using fn_t = std::function<std::pair<status, std::string>(context&, std::string_view, std::string_view)>;

    using services_t = std::unordered_map<std::string_view, fn_t>;

    template <typename S> 
    void add(S& s) 
    {
        fn_t fn = std::bind_front(&S::call, &s);
        m_services.insert({ s.name(), fn });
    }

protected:
    detail::response process(const detail::request &req) const noexcept;

private:
    services_t m_services;
};

} // namespace grpc_lite {}