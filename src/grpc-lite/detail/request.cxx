#include <grpc-lite/detail/Request.hxx>


namespace grpc_lite::detail
{

Request::Request(int32_t id) 
    : m_id(id) 
{
    GrpcLiteVerboseBlock("{}.Request::Request()", fmt::ptr(this));
}

Request::operator bool() const noexcept 
{
    return (!invalid() && !m_service.empty() && !m_method.empty());
}

void Request::header(std::string&& name, std::string&& value) noexcept 
{
    GrpcLiteVerboseBlock("{}.Request::header({}={})", fmt::ptr(this), name, value);

    // Avoid processing further if the request is already invalid
    if (invalid()) 
    {
        return;
    }

    if (name == ":method") 
    {
        flag(Flags::header_method);

        if (value != "POST") 
        {
            flag(Flags::invalid);
        }

        return;
    }

    if (name == ":path") 
    {
        flag(Flags::header_path);

        if (value.front() != '/') 
        {
            flag(Flags::invalid);
            return;
        }

        const auto n = value.find('/', 1);
        if (n == std::string::npos) 
        {
            flag(Flags::invalid);
            return;
        }

        m_service = value.substr(1, n - 1);
        m_method = value.substr(n + 1);
        if (m_method.empty()) 
        {
            flag(Flags::invalid);
            return;
        }

        return;
    }

    if (name == "content-type") 
    {
        flag(Flags::header_content_type);

        if (!value.starts_with("application/grpc")) 
        {
            flag(Flags::invalid);
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

bool Request::invalid() const noexcept 
{
    return flag(Flags::invalid);
}

void Request::read(const std::string_view data) noexcept 
{
    GrpcLiteVerboseBlock("{}.Request::read(len={})", fmt::ptr(this), data.length());

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
        flag(Flags::invalid);
    }
}

} // namespace grpc_lite::detail {}
