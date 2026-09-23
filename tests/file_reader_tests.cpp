#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "test_fixture.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

namespace
{
struct file_reader_fixture
{
    file_reader_fixture() : path{test_fixture::shared_reader_fixture_path()}
    {
        test_fixture::ensure_reader_fixture_file(path);
    }

    const std::filesystem::path path;
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
    REQUIRE(reader.tell() == 5); // peek must not advance
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader seek and skip clamp at end", "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    reader.seek(100);
    REQUIRE(reader.tell() == 10);
    REQUIRE(reader.eof());

    reader.seek(0);
    reader.skip(100);
    REQUIRE(reader.tell() == 10);
    REQUIRE(reader.eof());
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
    REQUIRE(reader.tell() == 0); // position unchanged
}

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at beyond end returns 0",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::array<std::byte, 4> slice{};
    REQUIRE(reader.read_at(slice, 10) == 0);
    REQUIRE(reader.read_at(slice, 100) == 0);
    REQUIRE(reader.tell() == 0); // position unchanged
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

    REQUIRE(reader.eof());

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

TEST_CASE_METHOD(file_reader_fixture, "file_reader read_at leaves stream position unchanged",
                 "[io][file_reader]")
{
    ripper::io::core::file_reader reader{path};

    std::array<std::byte, 3> slice{};
    std::ignore = reader.read_at(slice, 2); // reads bytes [2,3,4]

    REQUIRE(test_fixture::to_string(slice) == "234");
    REQUIRE(reader.tell() == 0); // read_at must not move the cursor
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

// ---------------------------------------------------------------------------
// read_line
// ---------------------------------------------------------------------------

TEST_CASE("file_reader read_line reads up to newline", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_test.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        const auto payload = test_fixture::to_bytes("hello\nworld");
        writer.write(payload);
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};

    const std::size_t n = reader.read_line(buf);

    REQUIRE(n == 5);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 5)) == "hello");
    REQUIRE(reader.tell() == 6); // includes consumed delimiter
}

TEST_CASE("file_reader read_line handles CR line ending", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_cr.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("alpha\rbeta"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};

    REQUIRE(reader.read_line(buf) == 5);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 5)) == "alpha");
    REQUIRE(reader.tell() == 6);

    REQUIRE(reader.read_line(buf) == 4);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 4)) == "beta");
    REQUIRE(reader.tell() == 10);
}

TEST_CASE("file_reader read_line handles CRLF line ending", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_crlf.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("alpha\r\nbeta"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};

    REQUIRE(reader.read_line(buf) == 5);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 5)) == "alpha");
    REQUIRE(reader.tell() == 7); // \r\n consumed

    REQUIRE(reader.read_line(buf) == 4);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 4)) == "beta");
    REQUIRE(reader.tell() == 11);
}

TEST_CASE("file_reader read_line handles empty lines", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_empty.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("\n\nx"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};

    REQUIRE(reader.read_line(buf) == 0);
    REQUIRE(reader.tell() == 1);

    REQUIRE(reader.read_line(buf) == 0);
    REQUIRE(reader.tell() == 2);

    REQUIRE(reader.read_line(buf) == 1);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 1)) == "x");
}

TEST_CASE("file_reader read_line stops at buffer capacity and defers delimiter",
          "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_capacity.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("abcdef\nrest"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 3> buf{};

    REQUIRE(reader.read_line(buf) == 3);
    REQUIRE(test_fixture::to_string(buf) == "abc");
    REQUIRE(reader.tell() == 3); // delimiter not yet consumed

    REQUIRE(reader.read_line(buf) == 3);
    REQUIRE(test_fixture::to_string(buf) == "def");
    REQUIRE(reader.tell() == 6);

    REQUIRE(reader.read_line(buf) == 0); // empty line, consumes '\n'
    REQUIRE(reader.tell() == 7);
}

TEST_CASE("file_reader read_line without trailing newline", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_readline_solo.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("solo"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    std::array<std::byte, 32> buf{};

    REQUIRE(reader.read_line(buf) == 4);
    REQUIRE(test_fixture::to_string(std::span{buf}.subspan(0, 4)) == "solo");
    REQUIRE(reader.tell() == 4);
    REQUIRE(reader.eof());
    REQUIRE(reader.read_line(buf) == 0);
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
        w.write(test_fixture::to_bytes("hello"));
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
    REQUIRE_THROWS(src.read_line(buf));  // NOLINT(bugprone-use-after-move)
}

TEST_CASE("file_reader peek at EOF throws", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_peek_eof.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes("0123456789"));
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};
    reader.seek(reader.size());

    REQUIRE(reader.eof());
    REQUIRE_THROWS(reader.peek());
}

TEST_CASE("file_reader empty input reports eof", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_empty_reader.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};

    REQUIRE(reader.size() == 0);
    REQUIRE(reader.eof());
    REQUIRE(reader.tell() == 0);

    std::array<std::byte, 4> buf{};
    REQUIRE(reader.read(buf) == 0);
    REQUIRE(reader.read_line(buf) == 0);
    REQUIRE(reader.read_at(buf, 0) == 0);
    REQUIRE_THROWS(reader.peek());
}

TEST_CASE("file_reader handles binary data with null bytes", "[io][file_reader]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_binary_reader.bin"};

    {
        ripper::io::core::file_writer writer{tmp.path()};
        const std::vector<std::byte> payload{std::byte{0}, std::byte{0}, std::byte{'A'}};
        writer.write(payload);
        writer.flush();
    }

    ripper::io::core::file_reader reader{tmp.path()};

    std::array<std::byte, 8> buf{};
    REQUIRE(reader.read(buf) == 3);
    REQUIRE(buf[0] == std::byte{0});
    REQUIRE(buf[1] == std::byte{0});
    REQUIRE(buf[2] == std::byte{'A'});
}