#include "echo.pb.h"

#include <grpc-lite/rpc.hxx>
#include <grpc-lite/server.hxx>
#include <grpc-lite/service.hxx>


#include <iostream>

namespace echo
{

using RpcEcho = grpc_lite::Rpc<"echo", echo::EchoRequest, echo::EchoResponse>;
using RpcSpam = grpc_lite::Rpc<"spam", echo::SpamRequest, echo::SpamResponse>;

using Service = grpc_lite::Service<"echo.Echo", RpcEcho, RpcSpam>;

struct EchoImpl 
{
    template <typename T>
    typename T::result_type call(grpc_lite::Context&, const typename T::request_type&) 
    {
        GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

        return { grpc_lite::Status::Code::unimplemented, std::nullopt };
    }

    std::uint32_t m_current = 0;
};


template <>
RpcEcho::result_type EchoImpl::call<RpcEcho>(grpc_lite::Context&, const EchoRequest& req)
{
    GrpcLiteVerboseBlock("{}.EchoImpl::call<RpcEcho>()", fmt::ptr(this));

    EchoResponse res;
    res.set_message("Hello `" + req.message());

    return { grpc_lite::Status::Code::ok, res };
}

template <>
RpcSpam::result_type EchoImpl::call<RpcSpam>(grpc_lite::Context&, const SpamRequest& req)
{
    GrpcLiteVerboseBlock("{}.EchoImpl::call<RpcSpam>()", fmt::ptr(this));

    SpamResponse res;
    res.set_current(m_current++);

    return { grpc_lite::Status::Code::ok, res };
}



} // namespace echo {}




int main(int argc, char** argv)
{
    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

    echo::EchoImpl impl;
    echo::Service service(impl);

    grpc_lite::Server server;
    server.add(service);

    std::cout << "Listening on [127.0.0.1:7000]...\n";
    server.run("127.0.0.1", 7000);

    return 0;
}