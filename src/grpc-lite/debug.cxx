#include <grpc-lite/debug.hxx>


#include <iostream>
#include <sstream>
#include <syncstream>


#if GRPC_LITE_WINDOWS
    #include <windows.h>
#elif GRPC_LITE_LINUX
    #define _GNU_SOURCE
    #include <unistd.h>
#endif

namespace grpc_lite::debug
{

namespace
{

void defaultTracer(Level level, std::uint32_t indent, std::string_view message)
{
    std::uintptr_t tid = 0;
#if GRPC_LITE_WINDOWS
    tid = ::GetCurrentThreadId();
#elif GRPC_LITE_LINUX
    tid = ::gettid();
#endif

    std::ostringstream ss;
    switch (level)
    {
    case grpc_lite::debug::Level::Verbose: ss << "V "; break;
    case grpc_lite::debug::Level::Info: ss << "I "; break;
    case grpc_lite::debug::Level::Error: ss << "E "; break;
    }

    ss << "@" << tid << " | ";

    while (indent)
    {
        ss << "    ";
        --indent;
    }

    ss << message << "\n";

    std::string msg(ss.str());

    if (level < Level::Error)
        std::osyncstream(std::cout) << msg;
    else
        std::osyncstream(std::cerr) << msg;

#if GRPC_LITE_WINDOWS
    if (::IsDebuggerPresent())
    {
        std::wstring out;

        auto required = ::MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), static_cast<int>(msg.length()), nullptr, 0);
        if (required > 0)
        {
            out.resize(required);
            ::MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), static_cast<int>(msg.length()), out.data(), static_cast<int>(out.size()));
        }

        ::OutputDebugStringW(out.c_str());
    }
#endif
}

TraceFn g_Tracer = defaultTracer;

thread_local std::uint32_t g_indent = 0;

constexpr std::uint32_t MaxIndent = 32;

} // namespace {}


void IndentScope::indent() noexcept
{
    if (g_indent < MaxIndent)
        ++g_indent;
}

void IndentScope::unindent() noexcept
{
    if (g_indent > 0)
        --g_indent;
}


GRPC_LITE_EXPORT TraceFn setTracer(TraceFn&& f)
{
    auto prev = std::move(g_Tracer);

    if (!f)
        g_Tracer = defaultTracer;
    else
        g_Tracer = std::move(f);

    return prev;
}

GRPC_LITE_EXPORT void writeln(Level level, std::string_view message)
{
    g_Tracer(level, g_indent, message);
}


} // namespace grpc_lite::debug {}
