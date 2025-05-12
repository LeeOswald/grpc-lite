#pragma once

#include <grpc-lite/grpc-lite.hxx>

#include <cstdint>
#include <optional>
#include <string>


namespace grpc_lite::detail
{

class GRPC_LITE_EXPORT message 
{
public:
    message() = default;
    message(std::string&& data);

    std::string bytes() const noexcept;
    void bytes(std::string_view bytes);

    std::string_view data() const noexcept;
    std::string_view prefix() const noexcept;

private:
    void parse();

    std::uint8_t m_compressed = 0;
    std::optional<std::uint32_t> m_length = std::nullopt;
    std::string m_data;
    std::string m_prefix;
};


} // namespace grpc_lite::detail {}
