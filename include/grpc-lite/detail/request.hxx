#pragma once

#include <grpc-lite/detail/message.hxx>

#include <cstdint>
#include <string>
#include <unordered_map>


namespace grpc_lite::detail
{

class GRPC_LITE_EXPORT Request 
{
public:
    using Metadata = std::unordered_map<std::string, std::string>;

    Request(Request&&) = default;
    Request(const Request&) = delete;
    
    Request(int32_t id);

    operator bool() const noexcept;

    std::int32_t id() const noexcept
    { 
        return m_id; 
    }

    std::string_view data() const noexcept 
    { 
        return m_msg.data(); 
    }

    const Metadata& metadata() const noexcept 
    { 
        return m_metadata; 
    }

    const std::string& method() const noexcept 
    { 
        return m_method; 
    }

    const std::string& service() const noexcept 
    { 
        return m_service; 
    }

    void setHeader(std::string&& name, std::string&& value) noexcept;

    bool invalid() const noexcept;

    void read(const std::string_view data) noexcept;

private:
    enum class Flags : std::uint8_t
    {
        invalid = 0x01,
        header_method = 0x02,          // :method
        header_path = 0x04,            // :path
        header_content_type = 0x08,    // content-type
    };

    void setFlag(Flags f) noexcept
    { 
        m_flags |= static_cast<std::uint8_t>(f);
    }

    bool hasFlag(Flags f) const noexcept 
    {
        return ((m_flags & static_cast<std::uint8_t>(f)) == static_cast<std::uint8_t>(f));
    }

    std::uint8_t m_flags = 0x00;
    std::int32_t m_id;
    Metadata m_metadata;
    Message m_msg;
    std::string m_method;
    std::string m_service;
};


} // namespace grpc_lite::detail {}
