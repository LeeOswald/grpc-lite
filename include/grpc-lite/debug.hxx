#pragma once

#ifndef GRPC_LITE_HXX_INCLUDED
#include <grpc-lite/grpc-lite.hxx>
#endif

#include <functional>
#include <string>

#include <fmt/format.h>


namespace grpc_lite::debug
{

enum class Level
{
    Verbose,
    Info,
    Error
};

using TraceFn = std::function<void(Level level, std::uint32_t indent, std::string_view message)>;


GRPC_LITE_EXPORT TraceFn setTracer(TraceFn&& f); // NOT thread-safe

GRPC_LITE_EXPORT void writeln(Level level, std::string_view message);

template <class... Args>
void write(Level level, std::string_view format, Args&&... args)
{
    writeln(level, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <class... Args>
void verbose(std::string_view format, Args&&... args)
{
    writeln(Level::Verbose, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <class... Args>
void info(std::string_view format, Args&&... args)
{
    writeln(Level::Info, fmt::vformat(format, fmt::make_format_args(args...)));
}

template <class... Args>
void error(std::string_view format, Args&&... args)
{
    writeln(Level::Error, fmt::vformat(format, fmt::make_format_args(args...)));
}


struct GRPC_LITE_EXPORT IndentScope
{
    ~IndentScope()
    {
        unindent();
    }

    template <class... Args>
    IndentScope(Level level, std::string_view format, Args&&... args)
    {
        writeln(level, fmt::vformat(format, fmt::make_format_args(args...)));

        indent();
    }

private:
    void indent() noexcept;
    void unindent() noexcept;
};


} // namespace grpc_lite::debug {}


#if GRPC_LITE_ENABLE_LOG

#if GRPC_LITE_VERBOSE_LOG

#define GrpcLiteVerbose(format, ...) \
    ::grpc_lite::debug::verbose(format, ##__VA_ARGS__)


#define  GrpcLiteVerboseBlock(format, ...) \
    ::grpc_lite::debug::IndentScope __is(::grpc_lite::debug::Level::Verbose, format, ##__VA_ARGS__)

#else

#define GrpcLiteVerbose(format, ...)                 ((void)0)
#define GrpcLiteVerboseBlock(format, ...)            ((void)0)

#endif

#define GrpcLiteInfo(format, ...) \
    ::grpc_lite::debug::info(format, ##__VA_ARGS__)

#define  GrpcLiteInfoBlock(format, ...) \
    ::grpc_lite::debug::IndentScope __is(::grpc_lite::debug::Level::Info, format, ##__VA_ARGS__)


#define GrpcLiteError(format, ...) \
    ::grpc_lite::debug::error(format, ##__VA_ARGS__)

#define  GrpcLiteErrorBlock(format, ...) \
    ::grpc_lite::debug::IndentScope __is(::grpc_lite::debug::Level::Error, format, ##__VA_ARGS__)

#else // !GRPC_LITE_ENABLE_LOG

#define GrpcLiteVerbose(format, ...)                 ((void)0)
#define GrpcLiteVerboseBlock(format, ...)            ((void)0)

#define GrpcLiteInfo(format, ...)                    ((void)0)
#define GrpcLiteInfoBlock(format, ...)               ((void)0)

#define GrpcLiteError(format, ...)                   ((void)0)
#define GrpcLiteErrorBlock(format, ...)              ((void)0)

#endif // !GRPC_LITE_ENABLE_LOG


#if defined __clang__ || defined __GNUC__
#define GRPC_LITE_FUNCTION __PRETTY_FUNCTION__
#elif defined _MSC_VER
#define GRPC_LITE_FUNCTION __FUNCSIG__
#endif