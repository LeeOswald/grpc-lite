#include <grpc-lite/detail/request.hxx>


namespace grpc_lite::detail
{

request::request(int32_t id) 
    : m_id(id) 
{
}

request::operator bool() const noexcept 
{
    return (!invalid() && !m_service.empty() && !m_method.empty());
}

void request::header(std::string&& name, std::string&& value) noexcept 
{
    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

    // Avoid processing further if the request is already invalid
    if (invalid()) 
    {
        return;
    }

    if (name == ":method") 
    {
        flag(flags_t::header_method);

        if (value != "POST") 
        {
            flag(flags_t::invalid);
        }

        return;
    }

    if (name == ":path") 
    {
        flag(flags_t::header_path);

        if (value.front() != '/') 
        {
            flag(flags_t::invalid);
            return;
        }

        const auto n = value.find('/', 1);
        if (n == std::string::npos) 
        {
            flag(flags_t::invalid);
            return;
        }

        m_service = value.substr(1, n - 1);
        m_method = value.substr(n + 1);
        if (m_method.empty()) 
        {
            flag(flags_t::invalid);
            return;
        }

        return;
    }

    if (name == "content-type") 
    {
        flag(flags_t::header_content_type);

        if (!value.starts_with("application/grpc")) 
        {
            flag(flags_t::invalid);
        }

        return;
    }

    // Keep a copy of "custom metadata" (i.e. headers that don't start with ':' or "grpc-")
    if (name.starts_with(':') || name.starts_with("grpc-")) 
    {
        return;
    }

    m_metadata.emplace(std::move(name), std::move(value));
}

bool request::invalid() const noexcept 
{
    return flag(flags_t::invalid);
}

void request::read(const std::string_view data) noexcept 
{
    GrpcLiteVerboseBlock("{}", GRPC_LITE_FUNCTION);

    if (invalid()) 
    {
        return;
    }

    try 
    {
        m_msg.bytes(data);
    }
    catch (...) 
    {
        flag(flags_t::invalid);
    }
}

} // namespace grpc_lite::detail {}
