#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <string>
#include <string_view>
#include <vector>


namespace grpc_lite::http2::detail
{

struct Header 
{
    std::string name;
    std::string value;
};

struct HeaderView 
{
    std::string_view name;
    std::string_view value;
};


using Headers = std::vector<HeaderView>;


} // namespace grpc_lite::http2::detail {}