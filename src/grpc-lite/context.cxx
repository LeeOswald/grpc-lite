#include <grpc-lite/context.hxx>
#include <grpc-lite/detail/request.hxx>


namespace grpc_lite
{

Context::Context(const detail::Request& req) noexcept 
{
    for (const auto& [key, value] : req.metadata()) 
    {
        m_meta.emplace(key, value);
    }
}

Context::Meta::mapped_type Context::meta(Meta::key_type key) const noexcept 
{
    const auto it = m_meta.find(key);
    if (it == m_meta.end()) 
    {
        return {};
    }

    return it->second;
}

} // namespace grpc_lite {}