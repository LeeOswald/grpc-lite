#include "grpc-lite.hpp"

#include <google/protobuf/compiler/plugin.h>

int main(int argc, char *argv[]) 
{
    Generator g;
    return google::protobuf::compiler::PluginMain(argc, argv, &g);
}
