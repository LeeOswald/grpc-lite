#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <string>
#include <string_view>
#include <vector>


namespace grpc_lite::http2::detail
{

struct header 
{
    std::string name;
    std::string value;
};

struct header_view 
{
    std::string_view name;
    std::string_view value;
};


using headers = std::vector<header_view>;


} // namespace grpc_lite::http2::detail {}