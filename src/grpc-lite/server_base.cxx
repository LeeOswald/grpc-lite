#include <grpc-lite/server_base.hxx>

#include <grpc-lite/detail/request.hxx>
#include <grpc-lite/detail/response.hxx>


namespace grpc_lite
{

detail::Response ServerBase::process(const detail::Request& req) const noexcept 
{
    GrpcLiteVerboseBlock("{}.ServerBase::process()", fmt::ptr(this));

    if (!req) 
    {
        return { req.id(), Status::Code::invalid_argument };
    }

    auto it = m_services.find(req.service());
    if (it == m_services.end()) 
    {
        return { req.id(), Status::Code::not_found };
    }

    Context ctx(req);
    detail::Response resp(req.id());

    try 
    {
        auto r = it->second(ctx, req.method(), req.data());
        resp.status(std::move(r.first));
        resp.data(std::move(r.second));
    }
    catch (std::exception& e) 
    {
        return { req.id(), Status::Code::internal };
    }

    return resp;
}

} // namespace grpc_lite {}
