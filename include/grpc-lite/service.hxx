#pragma once

#include <grpc-lite/context.hxx>
#include <grpc-lite/detail/fixed_string.hxx>
#include <grpc-lite/status.hxx>


#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>


namespace grpc_lite
{

namespace concepts 
{

template <typename T>
concept rpc_type = 
    requires(T t) 
{
    // Method
    T::method;
    requires std::same_as<std::remove_cv_t<decltype(T::method)>, std::string_view>;

    // Request
    typename T::request_type;
    { t.map(std::declval<std::string_view>()) } -> std::same_as<typename T::request_type>;

    // Response
    typename T::response_type;
    typename T::optional_response_type;
    {
        t.map(std::declval<const typename T::optional_response_type &>())
    } -> std::same_as<std::string>;

    // Result
    typename T::result_type;
        requires requires(typename T::result_type t) 
    {
            { t.status } -> std::same_as<grpc_lite::status&>;
            { t.response } -> std::same_as<typename T::optional_response_type &>;
    };
};

} // namespace concepts {}


template <fixed_string _Name, concepts::rpc_type... _Rpcs> 
class service 
{
public:
    using response_t = std::pair<status, std::string>;

    using handler_t = std::function<response_t(context&, std::string_view)>;
    using handlers_t = std::unordered_map<std::string_view, handler_t>;

    template <typename I> 
    constexpr explicit service(I& impl) 
    {
        GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

        std::apply(
            [&](auto&&...args) 
            {
                GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

                auto helper = [&](const auto& rpc) 
                {
                    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

                    auto handler = [&impl, &rpc](context& ctx, std::string_view data) -> response_t 
                    {
                        GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

                        using type = std::remove_cvref_t<decltype(rpc)>;

                        auto req = rpc.map(data);
                        auto result = std::invoke(&I::template call<type>, impl, ctx, req);

                        return { result.status, rpc.map(result.response) };
                    };

                    m_handlers.insert({ rpc.method, handler });
                };

                (helper(args), ...);
            },
            std::tuple<_Rpcs...>()
        );
    }

    response_t call(context &ctx, std::string_view method, std::string_view data) 
    {
        GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

        auto it = m_handlers.find(method);
        if (it == m_handlers.end()) 
        {
            return { status::code_t::not_found, {} };
        }

        return it->second(ctx, data);
    }

    constexpr std::string_view name() const noexcept 
    { 
        return { _Name }; 
    }

private:
    handlers_t m_handlers;
};


} // namespace grpc_lite {}