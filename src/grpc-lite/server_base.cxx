#include <grpc-lite/server_base.hxx>

#include <grpc-lite/detail/request.hxx>
#include <grpc-lite/detail/response.hxx>


namespace grpc_lite
{

detail::response server_base::process(const detail::request& req) const noexcept 
{
    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

    if (!req) 
    {
        return { req.id(), status::code_t::invalid_argument };
    }

    auto it = m_services.find(req.service());
    if (it == m_services.end()) 
    {
        return { req.id(), status::code_t::not_found };
    }

    context ctx(req);
    detail::response resp(req.id());

    try 
    {
        auto r = it->second(ctx, req.method(), req.data());
        resp.status(std::move(r.first));
        resp.data(std::move(r.second));
    }
    catch (std::exception& e) 
    {
        return { req.id(), status::code_t::internal };
    }

    return resp;
}

} // namespace grpc_lite {}
