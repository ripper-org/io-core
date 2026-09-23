#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/io/core/writer/memory_writer.hpp"
#include "test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <limits>
#include <span>
#include <vector>

namespace
{
struct memory_writer_fixture
{
    std::vector<std::byte> output;
};
} // namespace

// ---------------------------------------------------------------------------
// Basic writes
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer writes bytes to output vector",
                 "[io][memory_writer]")
{
    const std::vector<std::byte> payload = test_fixture::to_bytes("ripper-io-core");

    ripper::io::core::memory_writer writer{output};
    REQUIRE(writer.is_open());
    REQUIRE(writer.tell() == 0);

    writer.write(payload);
    REQUIRE(writer.tell() == payload.size());

    ripper::io::core::memory_reader reader{output};
    std::vector<std::byte> buffer(payload.size());

    const std::size_t bytesRead = reader.read(buffer);
    REQUIRE(bytesRead == payload.size());
    REQUIRE(test_fixture::to_string(buffer) == "ripper-io-core");
}

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer close prevents further writes",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    const std::vector<std::byte> data = test_fixture::to_bytes("abc");
    REQUIRE_THROWS(writer.write(data));
}

// ---------------------------------------------------------------------------
// Parameter validation / edge cases
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer write with empty span is a no-op",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};
    std::span<const std::byte> empty{};
    writer.write(empty);
    REQUIRE(writer.tell() == 0); // position unchanged
}

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer multiple sequential writes accumulate tell",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};

    const auto a = test_fixture::to_bytes("abc");
    const auto b = test_fixture::to_bytes("defgh");

    writer.write(a);
    REQUIRE(writer.tell() == 3);

    writer.write(b);
    REQUIRE(writer.tell() == 8);
    REQUIRE(test_fixture::to_string(output) == "abcdefgh");
}

TEST_CASE_METHOD(memory_writer_fixture,
                 "memory_writer seek then write places bytes at correct offset",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};

    writer.write(test_fixture::to_bytes("AAAAAAAAAA")); // 10 bytes
    writer.seek(3);
    writer.write(test_fixture::to_bytes("BB"));

    REQUIRE(test_fixture::to_string(output) == "AAABBAAAAA");
}

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer close is idempotent", "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    REQUIRE_NOTHROW(writer.close()); // second close must not throw
    REQUIRE_FALSE(writer.is_open());
}

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer seek on closed stream throws",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};
    writer.close();

    REQUIRE_THROWS(writer.seek(0));
}

TEST_CASE_METHOD(memory_writer_fixture, "memory_writer flush on closed stream throws",
                 "[io][memory_writer]")
{
    ripper::io::core::memory_writer writer{output};
    writer.close();

    REQUIRE_THROWS(writer.flush());
}

TEST_CASE("memory_writer vector-backed seek past end pads gap with zeros", "[io][memory_writer]")
{
    std::vector<std::byte> output;
    ripper::io::core::memory_writer writer{output};

    writer.seek(3);
    writer.write(test_fixture::to_bytes("XY"));

    REQUIRE(output.size() == 5);
    REQUIRE(output[0] == std::byte{0});
    REQUIRE(output[1] == std::byte{0});
    REQUIRE(output[2] == std::byte{0});
    REQUIRE(output[3] == std::byte{'X'});
    REQUIRE(output[4] == std::byte{'Y'});
}

TEST_CASE("memory_writer pointer constructor writes to target vector", "[io][memory_writer]")
{
    std::vector<std::byte> output;
    ripper::io::core::memory_writer writer{&output};

    const auto payload = test_fixture::to_bytes("ptr");
    writer.write(payload);

    REQUIRE(test_fixture::to_string(output) == "ptr");
}

TEST_CASE("memory_writer null pointer constructor throws", "[io][memory_writer]")
{
    std::vector<std::byte>* output = nullptr;
    REQUIRE_THROWS(ripper::io::core::memory_writer{output});
}

TEST_CASE("memory_writer move constructor detaches source", "[io][memory_writer]")
{
    std::vector<std::byte> output;
    ripper::io::core::memory_writer writer{output};
    writer.write(test_fixture::to_bytes("abc"));

    ripper::io::core::memory_writer moved{std::move(writer)};

    REQUIRE_FALSE(writer.is_open());
    REQUIRE(moved.is_open());
    REQUIRE(moved.tell() == 3);

    const auto data = test_fixture::to_bytes("def");
    REQUIRE_THROWS(writer.write(data)); // NOLINT(bugprone-use-after-move)

    moved.write(data);
    REQUIRE(test_fixture::to_string(output) == "abcdef");
}

TEST_CASE("memory_writer move assignment detaches source", "[io][memory_writer]")
{
    std::vector<std::byte> output_a;
    std::vector<std::byte> output_b;

    ripper::io::core::memory_writer a{output_a};
    a.write(test_fixture::to_bytes("x"));

    ripper::io::core::memory_writer b{output_b};
    b = std::move(a);

    REQUIRE_FALSE(a.is_open()); // NOLINT(bugprone-use-after-move)
    REQUIRE(b.is_open());

    b.write(test_fixture::to_bytes("y"));
    REQUIRE(test_fixture::to_string(output_a) == "xy"); // b now targets a's buffer
    REQUIRE(output_b.empty());
}

TEST_CASE("memory_writer write at maximum position overflows", "[io][memory_writer]")
{
    std::vector<std::byte> output;
    ripper::io::core::memory_writer writer{output};

    writer.seek(std::numeric_limits<std::size_t>::max());

    REQUIRE_THROWS_AS(writer.write(test_fixture::to_bytes("x")), std::overflow_error);
}
