#include "core/util/numeric_cast.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>

using ripper::io::core::utils::checked_narrow;

// ---------------------------------------------------------------------------
// Valid conversions
// ---------------------------------------------------------------------------

TEST_CASE("checked_narrow: unsigned within signed range succeeds", "[util][numeric_cast]")
{
    constexpr std::size_t value = 1024;
    REQUIRE(checked_narrow<std::streamsize>(value, "size") == static_cast<std::streamsize>(1024));
}

TEST_CASE("checked_narrow: uint64 within streamoff range succeeds", "[util][numeric_cast]")
{
    constexpr std::uint64_t value = 4096;
    REQUIRE(checked_narrow<std::streamoff>(value, "offset") == static_cast<std::streamoff>(4096));
}

TEST_CASE("checked_narrow: zero converts cleanly", "[util][numeric_cast]")
{
    REQUIRE(checked_narrow<std::streamsize>(std::size_t{0}, "zero") == 0);
    REQUIRE(checked_narrow<std::streamoff>(std::uint64_t{0}, "zero") == 0);
}

TEST_CASE("checked_narrow: max boundary value succeeds", "[util][numeric_cast]")
{
    constexpr auto max_ok = static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max());
    REQUIRE(checked_narrow<std::streamsize>(max_ok, "max") ==
            std::numeric_limits<std::streamsize>::max());
}

// ---------------------------------------------------------------------------
// Overflow cases (no file needed)
// ---------------------------------------------------------------------------

TEST_CASE("checked_narrow: unsigned value exceeding signed max throws", "[util][numeric_cast]")
{
    const std::size_t overflow_value =
        static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()) + 1;
    REQUIRE_THROWS_AS(checked_narrow<std::streamsize>(overflow_value, "overflow"),
                      std::runtime_error);
}

TEST_CASE("checked_narrow: uint64 exceeding streamoff max throws", "[util][numeric_cast]")
{
    const std::uint64_t overflow_value =
        static_cast<std::uint64_t>(std::numeric_limits<std::streamoff>::max()) + 1;
    REQUIRE_THROWS_AS(checked_narrow<std::streamoff>(overflow_value, "overflow"),
                      std::runtime_error);
}

TEST_CASE("checked_narrow: negative signed to unsigned throws", "[util][numeric_cast]")
{
    REQUIRE_THROWS_AS(checked_narrow<std::size_t>(std::int64_t{-1}, "negative"),
                      std::runtime_error);
}

TEST_CASE("checked_narrow: large unsigned to small unsigned throws", "[util][numeric_cast]")
{
    const std::uint64_t big =
        static_cast<std::uint64_t>(std::numeric_limits<std::uint8_t>::max()) + 1;
    REQUIRE_THROWS_AS(checked_narrow<std::uint8_t>(big, "too big"), std::runtime_error);
}

TEST_CASE("checked_narrow: error message contains operation name", "[util][numeric_cast]")
{
    const std::size_t overflow_value =
        static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()) + 1;
    try
    {
        std::ignore = checked_narrow<std::streamsize>(overflow_value, "my_operation");
        FAIL("expected exception");
    }
    catch (const std::runtime_error& e)
    {
        REQUIRE(std::string_view{e.what()}.find("my_operation") != std::string_view::npos);
    }
}
