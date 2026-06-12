#include "ripper/io/core/reader/memory_reader.hpp"
#include "test_fixture.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>
#include <vector>

namespace
{
struct memory_reader_fixture
{
    std::vector<std::byte> payload = test_fixture::to_bytes("0123456789");
};
} // namespace

// ---------------------------------------------------------------------------
// Basic reads
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader reads entire fixture payload",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};
    REQUIRE(reader.is_open());
    REQUIRE(reader.size() == 10);

    std::vector<std::byte> buffer(10);
    const std::size_t bytesRead = reader.read(buffer);

    REQUIRE(bytesRead == buffer.size());
    REQUIRE(test_fixture::to_string(buffer) == "0123456789");
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader read_at returns expected window",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    std::array<std::byte, 4> slice{};
    const std::size_t bytesRead = reader.read_at(slice, 3);

    REQUIRE(bytesRead == slice.size());
    REQUIRE(test_fixture::to_string(slice) == "3456");
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader seek and peek are consistent",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    reader.seek(5);
    REQUIRE(reader.tell() == 5);
    REQUIRE(static_cast<char>(reader.peek()) == '5');
}

// ---------------------------------------------------------------------------
// Parameter validation / edge cases
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader read with empty span returns 0",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};
    std::span<std::byte> empty{};
    REQUIRE(reader.read(empty) == 0);
    REQUIRE(reader.tell() == 0); // position unchanged
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader read_at with empty span returns 0",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};
    std::span<std::byte> empty{};
    REQUIRE(reader.read_at(empty, 5) == 0);
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader skip advances position",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    reader.skip(3);
    REQUIRE(reader.tell() == 3);

    reader.skip(4);
    REQUIRE(reader.tell() == 7);
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader eof is true after reading all bytes",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    std::vector<std::byte> buffer(10);
    std::ignore = reader.read(buffer);

    std::array<std::byte, 1> extra{};
    std::ignore = reader.read(extra);

    REQUIRE(reader.eof());
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader read past EOF returns fewer bytes",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    reader.seek(8); // 2 bytes remaining

    std::vector<std::byte> buffer(10); // request more than available
    const std::size_t bytesRead = reader.read(buffer);

    REQUIRE(bytesRead == 2);
    REQUIRE(test_fixture::to_string(std::span{buffer}.subspan(0, 2)) == "89");
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader read_at leaves stream position after window",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    std::array<std::byte, 3> slice{};
    std::ignore = reader.read_at(slice, 2); // reads bytes [2,3,4]

    REQUIRE(reader.tell() == 5); // seeked to 2, read 3 -> position is 5
}

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader seek to beginning resets position",
                 "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};

    std::vector<std::byte> buf1(5);
    std::ignore = reader.read(buf1);
    REQUIRE(reader.tell() == 5);

    reader.seek(0);
    REQUIRE(reader.tell() == 0);

    std::vector<std::byte> buf2(5);
    std::ignore = reader.read(buf2);
    REQUIRE(test_fixture::to_string(buf1) == test_fixture::to_string(buf2));
}

TEST_CASE("memory_reader read_line reads up to newline", "[io][memory_reader]")
{
    const auto payload = test_fixture::to_bytes("hello\nworld");
    ripper::io::core::memory_reader reader{payload};

    std::array<std::byte, 32> buf{};
    const std::size_t n = reader.read_line(buf);

    REQUIRE(n == 5);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 5)) == "hello");
    REQUIRE(reader.tell() == 6); // includes consumed delimiter
}

// ---------------------------------------------------------------------------
// Error cases
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(memory_reader_fixture, "memory_reader peek at EOF throws", "[io][memory_reader]")
{
    ripper::io::core::memory_reader reader{payload};
    reader.seek(reader.size());

    REQUIRE_THROWS(reader.peek());
}
