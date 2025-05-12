#pragma once

#include <grpc-lite/detail/message.hxx>

#include <cstdint>
#include <string>
#include <unordered_map>


namespace grpc_lite::detail
{

class GRPC_LITE_EXPORT request 
{
public:
    using metadata_t = std::unordered_map<std::string, std::string>;

    request(request&&) = default;
    request(const request&) = delete;
    
    request(int32_t id);

    operator bool() const noexcept;

    int32_t id() const noexcept 
    { 
        return m_id; 
    }

    std::string_view data() const noexcept 
    { 
        return m_msg.data(); 
    }

    const metadata_t& metadata() const noexcept 
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

    void header(std::string&& name, std::string&& value) noexcept;

    bool invalid() const noexcept;

    void read(const std::string_view data) noexcept;

private:
    enum class flags_t : uint8_t 
    {
        invalid = 0x01,
        header_method = 0x02,          // :method
        header_path = 0x04,            // :path
        header_content_type = 0x08,    // content-type
    };

    void flag(flags_t f) noexcept 
    { 
        m_flags |= static_cast<uint8_t>(f); 
    }

    bool flag(flags_t f) const noexcept 
    {
        return ((m_flags & static_cast<uint8_t>(f)) == static_cast<uint8_t>(f));
    }

    uint8_t m_flags = 0x00;
    int32_t m_id;
    metadata_t m_metadata;
    message m_msg;
    std::string m_method;
    std::string m_service;
};


} // namespace grpc_lite::detail {}
