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
concept IsRpc = 
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
            { t.status } -> std::same_as<grpc_lite::Status&>;
            { t.response } -> std::same_as<typename T::optional_response_type &>;
    };
};

} // namespace concepts {}


template <FixedString _Name, concepts::IsRpc... _Rpcs> 
class Service 
{
public:
    using Response = std::pair<Status, std::string>;

    using Handler = std::function<Response(Context&, std::string_view)>;
    using Handlers = std::unordered_map<std::string_view, Handler>;

    template <typename I> 
    constexpr explicit Service(I& impl) 
    {
        GrpcLiteVerboseBlock("{}.Service<{}>::Service()", fmt::ptr(this), std::string_view{_Name});

        std::apply(
            [&](auto&&...args) 
            {
                GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

                auto helper = [&](const auto& rpc) 
                {
                    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

                    auto handler = [&impl, &rpc](Context& ctx, std::string_view data) -> Response 
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

    Response call(Context& ctx, std::string_view method, std::string_view data) 
    {
        GrpcLiteVerboseBlock("{}.Service<{}>::call(method={})", fmt::ptr(this), std::string_view{ _Name }, method);

        auto it = m_handlers.find(method);
        if (it == m_handlers.end()) 
        {
            return { Status::Code::not_found, {} };
        }

        return it->second(ctx, data);
    }

    constexpr std::string_view name() const noexcept 
    { 
        return { _Name }; 
    }

private:
    Handlers m_handlers;
};


} // namespace grpc_lite {}