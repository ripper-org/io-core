#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ripper::io::core::utils
{
template <typename To, typename From>
[[nodiscard]] constexpr To checked_narrow(const From value,
                                          const std::string_view operation)
{
    static_assert(std::is_integral_v<To>, "checked_narrow requires integral target type");
    static_assert(std::is_integral_v<From>, "checked_narrow requires integral source type");

    if (!std::in_range<To>(value))
    {
        throw std::runtime_error{std::string{operation} + " exceeds target range"};
    }

    return static_cast<To>(value);
}

} // namespace ripper::io::core::utils