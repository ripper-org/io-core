#include "core/reader/file_reader.hpp"
#include "core/writer/file_writer.hpp"
#include "test_fixture.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <filesystem>
#include <vector>

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
} // namespace

// ---------------------------------------------------------------------------
// Basic reads
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(file_reader_fixture, "file_reader reads entire fixture payload",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};
    REQUIRE(reader.is_open());
    REQUIRE(reader.size() == 10);

    std::vector<std::byte> buffer(10);
    const std::size_t bytesRead = reader.read(buffer);

    REQUIRE(bytesRead == buffer.size());
    REQUIRE(test_fixture::to_string(buffer) == "0123456789");
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at returns expected window",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::array<std::byte, 4> slice{};
    const std::size_t bytesRead = reader.read_at(slice, 3);

    REQUIRE(bytesRead == slice.size());
    REQUIRE(test_fixture::to_string(slice) == "3456");
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader seek and peek are consistent",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    reader.seek(5);
    REQUIRE(reader.tell() == 5);
    REQUIRE(static_cast<char>(reader.peek()) == '5');
}

// ---------------------------------------------------------------------------
// Parameter validation / edge cases
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(file_reader_fixture, "file_reader read with empty span returns 0",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};
    std::span<std::byte> empty{};
    REQUIRE(reader.read(empty) == 0);
    REQUIRE(reader.tell() == 0); // position unchanged
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at with empty span returns 0",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};
    std::span<std::byte> empty{};
    REQUIRE(reader.read_at(empty, 5) == 0);
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader skip advances position", "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    reader.skip(3);
    REQUIRE(reader.tell() == 3);

    reader.skip(4);
    REQUIRE(reader.tell() == 7);
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader eof is true after reading all bytes",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::vector<std::byte> buffer(10);
    std::ignore = reader.read(buffer);

    // Trigger eof state by attempting one more byte read
    std::array<std::byte, 1> extra{};
    std::ignore = reader.read(extra);

    REQUIRE(reader.eof());
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read past EOF returns fewer bytes",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    reader.seek(8); // 2 bytes remaining

    std::vector<std::byte> buffer(10); // request more than available
    const std::size_t bytesRead = reader.read(buffer);

    REQUIRE(bytesRead == 2);
    REQUIRE(test_fixture::to_string(std::span{buffer}.subspan(0, 2)) == "89");
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at leaves stream position after window",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::array<std::byte, 3> slice{};
    std::ignore = reader.read_at(slice, 2); // reads bytes [2,3,4]

    REQUIRE(reader.tell() == 5); // seeked to 2, read 3 → position is 5
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader seek to beginning resets position",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::vector<std::byte> buf1(5);
    std::ignore = reader.read(buf1);
    REQUIRE(reader.tell() == 5);

    reader.seek(0);
    REQUIRE(reader.tell() == 0);

    std::vector<std::byte> buf2(5);
    std::ignore = reader.read(buf2);
    REQUIRE(test_fixture::to_string(buf1) == test_fixture::to_string(buf2));
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_line reads up to newline",
                 "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_test.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        const auto payload = test_fixture::to_bytes("hello\nworld");
        std::ignore = writer.write(payload);
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};
    const std::size_t n = reader.read_line(buf);

    // gcount includes the delimiter that was consumed but not stored
    REQUIRE(n >= 5);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 5)) == "hello");
}

// ---------------------------------------------------------------------------
// Error cases
// ---------------------------------------------------------------------------

TEST_CASE("file_reader opening nonexistent path throws", "[io][file_reader]")
{
    REQUIRE_THROWS(
        ripper::io::core::file_reader{std::filesystem::path{"/nonexistent/no_such_file.bin"}});
}

TEST_CASE("file_reader operations on moved-from instance throw", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_closed_reader_test.bin"};

    {
        ripper::io::core::file_writer w{tmp.path()};
        std::ignore = w.write(test_fixture::to_bytes("hello"));
        w.flush();
    }

    ripper::io::core::file_reader src{tmp.path()};
    ripper::io::core::file_reader moved{std::move(src)};

    // src is now in moved-from state: stream is not open, all ops must throw
    REQUIRE_FALSE(src.is_open()); // NOLINT(bugprone-use-after-move)

    std::vector<std::byte> buf(4);
    REQUIRE_THROWS(src.read(buf));       // NOLINT(bugprone-use-after-move)
    REQUIRE_THROWS(src.read_at(buf, 0)); // NOLINT(bugprone-use-after-move)
    REQUIRE_THROWS(src.seek(0));         // NOLINT(bugprone-use-after-move)
    REQUIRE_THROWS(src.skip(1));         // NOLINT(bugprone-use-after-move)
    REQUIRE_THROWS(src.peek());          // NOLINT(bugprone-use-after-move)
}
