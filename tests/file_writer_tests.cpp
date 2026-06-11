#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/reader/file_reader.hpp"
#include "core/writer/file_writer.hpp"
#include "test_fixture.hpp"

namespace
{
    struct file_writer_fixture
    {
        test_fixture::scoped_temp_file output{"io_ripper_core_writer_test.bin"};
    };
}

TEST_CASE_METHOD(file_writer_fixture, "file_writer writes bytes to output file", "[io][file_writer]")
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

TEST_CASE_METHOD(file_writer_fixture, "file_writer close prevents further writes", "[io][file_writer]")
{
    ripper::io::core::file_writer writer{output.path()};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    const std::vector<std::byte> data = test_fixture::to_bytes("abc");
    REQUIRE_THROWS(writer.write(data));
}
