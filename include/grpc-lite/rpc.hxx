#pragma once

#include <grpc-lite/detail/fixed_string.hxx>
#include <grpc-lite/status.hxx>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace grpc_lite
{


template <fixed_string _Method, typename _Request, typename _Response> 
struct rpc 
{
    static constexpr std::string_view method{ _Method };

    using method_type = fixed_string_t<_Method>;
    using request_type = _Request;
    using response_type = _Response;
    using optional_response_type = std::optional<response_type>;

    struct result_type 
    {
        class status status;
        optional_response_type response;
    };

    request_type map(std::string_view data) const 
    {
        constexpr bool can_map = requires(request_type t) 
        {
            { t.ParseFromArray(std::declval<const char*>(), std::declval<std::size_t>()) } -> std::same_as<bool>;
        };
        
        static_assert(can_map, "No known method to deserialize data");

        request_type req;

        if (!req.ParseFromArray(data.data(), data.size())) 
        {
            throw std::runtime_error("Failed to deserialize data");
        }

        return req;
    }

    std::string map(const optional_response_type& res) const 
    {
        constexpr bool can_map = requires(response_type t) 
        {
            { t.SerializeToString(std::declval<std::string *>()) } -> std::same_as<bool>;
        };

        static_assert(can_map, "No known method to serialize data");

        if (!res) 
        {
            return {};
        }

        std::string data;
        if (!res->SerializeToString(&data)) 
        {
            throw std::runtime_error("Failed to serialize data");
        }

        return data;
    }
};


} // namespace grpc_lite {}
