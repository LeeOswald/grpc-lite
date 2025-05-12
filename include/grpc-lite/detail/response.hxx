#pragma once

#include <grpc-lite/detail/message.hxx>
#include <grpc-lite/status.hxx>

#include <cstdint>
#include <string>


namespace grpc_lite::detail
{

class GRPC_LITE_EXPORT response 
{
public:
    response(int32_t id, status::code_t code = status::code_t::ok) 
        : m_id(id)
        , m_status(code) 
    {
    }

    int32_t id() const noexcept 
    { 
        return m_id; 
    }

    std::string bytes() const noexcept 
    { 
        return m_msg.bytes(); 
    }

    const auto& status() noexcept 
    { 
        return m_status; 
    }

    void data(std::string&& d) noexcept 
    { 
        m_msg = std::move(d); 
    }

    void status(class status&& s) noexcept 
    { 
        m_status = std::move(s); 
    }

private:
    int32_t m_id;
    message m_msg;
    class status m_status;
};


} // namespace grpc_lite::detail {}