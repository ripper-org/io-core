#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

namespace
{
struct file_writer_fixture
{
    test_fixture::scoped_temp_file output{"io_ripper_core_writer_test.bin"};
};
} // namespace

// ---------------------------------------------------------------------------
// Basic writes
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(file_writer_fixture, "file_writer writes bytes to output file",
                 "[io][file_writer]")
{
    const std::vector<std::byte> payload = test_fixture::to_bytes("ripper-io-core");

    {
        ripper::io::core::file_writer writer{output.path()};
        REQUIRE(writer.is_open());
        REQUIRE(writer.tell() == 0);

        const std::size_t written = writer.write(payload);
        REQUIRE(written == payload.size());
        REQUIRE(writer.tell() == payload.size());

        writer.flush();
    }

    {
        ripper::io::core::file_reader reader{output.path()};
        std::vector<std::byte> buffer(payload.size());

        const std::size_t bytesRead = reader.read(buffer);
        REQUIRE(bytesRead == payload.size());
        REQUIRE(test_fixture::to_string(buffer) == "ripper-io-core");
    }
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer close prevents further writes",
                 "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    const std::vector<std::byte> data = test_fixture::to_bytes("abc");
    REQUIRE_THROWS(writer.write(data));
}

// ---------------------------------------------------------------------------
// Parameter validation / edge cases
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(file_writer_fixture, "file_writer write with empty span returns 0",
                 "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    std::span<const std::byte> empty{};
    REQUIRE(writer.write(empty) == 0);
    REQUIRE(writer.tell() == 0); // position unchanged
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer multiple sequential writes accumulate tell",
                 "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};

    const auto a = test_fixture::to_bytes("abc");
    const auto b = test_fixture::to_bytes("defgh");

    std::ignore = writer.write(a);
    REQUIRE(writer.tell() == 3);

    std::ignore = writer.write(b);
    REQUIRE(writer.tell() == 8);
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer seek then write places bytes at correct offset",
                 "[io][file_writer]")
{
    {
        ripper::io::core::file_writer writer{output.path()};
        std::ignore = writer.write(test_fixture::to_bytes("AAAAAAAAAA")); // 10 bytes
        writer.seek(3);
        std::ignore = writer.write(test_fixture::to_bytes("BB"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{output.path()};
    std::vector<std::byte> buf(10);
    std::ignore = reader.read(buf);
    REQUIRE(test_fixture::to_string(buf) == "AAABBAAAAA");
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer close is idempotent", "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    REQUIRE_NOTHROW(writer.close()); // second close must not throw
    REQUIRE_FALSE(writer.is_open());
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer seek on closed stream throws",
                 "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    writer.close();

    REQUIRE_THROWS(writer.seek(0));
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer flush on closed stream throws",
                 "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    writer.close();

    REQUIRE_THROWS(writer.flush());
}
