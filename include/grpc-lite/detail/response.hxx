#pragma once

#include <grpc-lite/detail/message.hxx>
#include <grpc-lite/status.hxx>

#include <cstdint>
#include <string>


namespace grpc_lite::detail
{

class GRPC_LITE_EXPORT Response 
{
public:
    Response(std::int32_t id, Status::Code code = Status::Code::ok)
        : m_id(id)
        , m_status(code) 
    {
    }

    std::int32_t id() const noexcept
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

    void setData(std::string&& d) noexcept 
    { 
        m_msg = std::move(d); 
    }

    void setStatus(class Status&& s) noexcept 
    { 
        m_status = std::move(s); 
    }

private:
    std::int32_t m_id;
    Message m_msg;
    class Status m_status;
};


} // namespace grpc_lite::detail {}