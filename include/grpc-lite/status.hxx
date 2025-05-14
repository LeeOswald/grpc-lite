#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <string>
#include <string_view>


namespace grpc_lite
{

class GRPC_LITE_EXPORT Status 
{
public:
    enum class Code : std::int8_t
    {
        ok = 0,
        cancelled = 1,
        unknown = 2,
        invalid_argument = 3,
        deadline_exceeded = 4,
        not_found = 5,
        already_exists = 6,
        permission_denied = 7,
        resource_exhausted = 8,
        failed_precondition = 9,
        aborted = 10,
        out_of_range = 11,
        unimplemented = 12,
        internal = 13,
        unavailable = 14,
        data_loss = 15,
        unauthenticated = 16,
    };

    constexpr Status(Code code = Code::ok) noexcept
        : m_code(code)
        , m_str() 
    {
    }

    Status(Code code, std::string&& details) noexcept
        : m_code(code)
        , m_details(std::move(details))
        , m_str() 
    {
    }

    operator std::string_view() const noexcept
    { 
        return str(); 
    }

    constexpr Code code() const noexcept 
    { 
        return m_code; 
    }

    const std::string& details() const noexcept 
    { 
        return m_details; 
    }

    std::string_view str() const 
    {
        if (m_str.empty()) 
        {
            m_str = std::to_string(static_cast<std::int8_t>(m_code));
        }

        return m_str;
    }

private:
    Code m_code;
    std::string m_details;
    mutable std::string m_str;
};


} // namespace grpc_lite {}
