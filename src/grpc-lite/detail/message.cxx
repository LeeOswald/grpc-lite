#include <grpc-lite/detail/message.hxx>

#include <array>
#include <stdexcept>



namespace grpc_lite::detail
{

Message::Message(std::string&& data) 
    : m_data(std::move(data))
    , m_length(static_cast<std::uint32_t>(data.size())) 
{
    GrpcLiteVerboseBlock("{}.Message::Message()", fmt::ptr(this));

    std::uint32_t size = m_length.value();

    std::array<char, 5> bytes;
    bytes[0] = 0x00;
    bytes[1] = (size >> 24) & 0xff;
    bytes[2] = (size >> 16) & 0xff;
    bytes[3] = (size >> 8) & 0xff;
    bytes[4] = size & 0xff;

    m_prefix = { bytes.data(), bytes.size() };
}

std::string Message::bytes() const noexcept 
{
    return m_prefix + m_data;
}

void Message::bytes(std::string_view bytes) 
{
    GrpcLiteVerboseBlock("{}.Message::bytes(len={})", fmt::ptr(this), bytes.length());

    if (m_length && m_data.size() >= m_length.value()) 
    {
        throw std::length_error("Message larger than what's indicated in the prefix.");
    }

    std::size_t head = 0;
    if (m_prefix.size() < 5) 
    {
        std::size_t n = bytes.size() > 5 ? 5 : bytes.size();
        m_prefix.append(bytes.data(), n);

        head += n;
    }

    parse();

    if (bytes.size() - head > 0) 
    {
        m_data.append(bytes.data() + head, bytes.size() - head);
    }
}

std::string_view Message::data() const noexcept 
{
    if (!m_length || (m_data.size() != m_length.value())) 
    {
        return {};
    }

    return m_data;
}

void Message::parse() 
{
    GrpcLiteVerboseBlock("{}.Message::parse()", fmt::ptr(this));

    if (m_length) 
    {
        // Already parsed
        return;
    }

    if (m_prefix.size() < 5) 
    {
        // Not enough bytes to parse
        return;
    }

    m_compressed = m_prefix[0];
    m_length = std::make_optional((m_prefix[1] << 24) | (m_prefix[2] << 16) | (m_prefix[3] << 8) | (m_prefix[4] & 0xff));

    const std::size_t cap = m_length.value() + 5;
    if (m_data.capacity() < cap) 
    {
        m_data.reserve(cap);
    }
}

std::string_view Message::prefix() const noexcept 
{
    return m_prefix;
}


} // namespace grpc_lite::detail {}
