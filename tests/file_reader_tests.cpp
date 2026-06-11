#include <array>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/reader/file_reader.hpp"
#include "core/writer/file_writer.hpp"
#include "test_fixture.hpp"

namespace
{
    struct file_reader_fixture
    {
        file_reader_fixture()
        {
            test_fixture::ensure_reader_fixture_file();
        }

        const std::filesystem::path path = test_fixture::shared_reader_fixture_path();
    };
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader reads entire fixture payload", "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};
    REQUIRE(reader.is_open());
    REQUIRE(reader.size() == 10);

    std::vector<std::byte> buffer(10);
    const std::size_t bytesRead = reader.read(buffer);

    REQUIRE(bytesRead == buffer.size());
    REQUIRE(test_fixture::to_string(buffer) == "0123456789");
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at returns expected window", "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::array<std::byte, 4> slice{};
    const std::size_t bytesRead = reader.read_at(slice, 3);

    REQUIRE(bytesRead == slice.size());
    REQUIRE(test_fixture::to_string(slice) == "3456");
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader seek and peek are consistent", "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    reader.seek(5);
    REQUIRE(reader.tell() == 5);
    REQUIRE(static_cast<char>(reader.peek()) == '5');
}
