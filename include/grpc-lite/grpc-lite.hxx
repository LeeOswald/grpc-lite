#pragma once


#if GRPC_LITE_WINDOWS
    #ifdef GRPC_LITE_EXPORTS
        #define GRPC_LITE_EXPORT __declspec(dllexport)
    #else
        #define GRPC_LITE_EXPORT __declspec(dllimport)
    #endif
#else
    #define GRPC_LITE_EXPORT __attribute__((visibility("default")))
#endif

#define GRPC_LITE_HXX_INCLUDED 1

#include <grpc-lite/debug.hxx>