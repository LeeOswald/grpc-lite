#pragma once

#include <algorithm>
#include <string_view>


namespace grpc_lite
{

namespace detail
{

template <class CharT, std::size_t N> 
struct BasicFixedString 
{
    using value_type = CharT;

    constexpr BasicFixedString(const value_type(&str)[N + 1]) noexcept 
    {
        std::copy_n(str, N + 1, value);
    }

    constexpr operator std::basic_string_view<value_type>() const noexcept 
    { 
        return view(); 
    }

    template <std::size_t M>
    constexpr auto operator==(const BasicFixedString<value_type, M>& rhs) const noexcept 
    {
        return (N == M && view() == rhs.view());
    }

    [[nodiscard]] constexpr const value_type* data() const noexcept 
    { 
        return &value[0]; 
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept 
    { 
        return N; 
    }

    [[nodiscard]] constexpr std::basic_string_view<value_type> view() const noexcept 
    {
        return { &value[0], N };
    }

    value_type value[N + 1]; // extra space for null terminations when constructing from 'const ChatT *'
};


} // namespace detail {}


template <std::size_t N> 
struct FixedString 
    : detail::BasicFixedString<char, N - 1> 
{
    constexpr FixedString(const char(&str)[N]) noexcept 
        : detail::BasicFixedString<char, N - 1>(str) 
    {}
};


template <FixedString T> 
struct StringLiteral 
{
};


} // namespace grpc_lite {}