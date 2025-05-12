#include <grpc-lite/context.hxx>
#include <grpc-lite/detail/request.hxx>


namespace grpc_lite
{

context::context(const detail::request& req) noexcept 
{
    for (const auto& [key, value] : req.metadata()) 
    {
        m_meta.emplace(key, value);
    }
}

context::meta_t::mapped_type context::meta(meta_t::key_type key) const noexcept 
{
    const auto it = m_meta.find(key);
    if (it == m_meta.end()) 
    {
        return {};
    }

    return it->second;
}

} // namespace grpc_lite {}