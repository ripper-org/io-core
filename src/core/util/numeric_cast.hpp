#pragma once

#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace ripper::io::core::utils
{
template <typename To, typename From>
[[nodiscard]] constexpr To checked_narrow(const From value,
                                          const std::string_view operation)
{
    static_assert(std::is_integral_v<To>, "checked_narrow requires integral target type");
    static_assert(std::is_integral_v<From>, "checked_narrow requires integral source type");

    constexpr auto to_min = std::numeric_limits<To>::lowest();
    constexpr auto to_max = std::numeric_limits<To>::max();

    if constexpr (std::is_signed_v<From> && std::is_signed_v<To>)
    {
        if (value < static_cast<From>(to_min) || value > static_cast<From>(to_max))
        {
            throw std::runtime_error{std::string{operation} + " exceeds target range"};
        }
    }
    else if constexpr (std::is_signed_v<From> && !std::is_signed_v<To>)
    {
        if (value < 0 || value > static_cast<From>(to_max))
        {
            throw std::runtime_error{std::string{operation} + " exceeds target range"};
        }
    }
    else if constexpr (!std::is_signed_v<From> && std::is_signed_v<To>)
    {
        if (value > static_cast<std::make_unsigned_t<To>>(to_max))
        {
            throw std::runtime_error{std::string{operation} + " exceeds target range"};
        }
    }
    else
    {
        if (value > static_cast<From>(to_max))
        {
            throw std::runtime_error{std::string{operation} + " exceeds target range"};
        }
    }

    return static_cast<To>(value);
}

} // namespace ripper::io::core::utils
