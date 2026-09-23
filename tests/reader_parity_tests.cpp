#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "test_fixture.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace
{
std::string read_text(ripper::io::core::reader& reader, std::string& out,
                      const std::size_t capacity)
{
    std::vector<std::byte> buffer(capacity);
    const std::size_t n = reader.read(buffer);
    out = test_fixture::to_string(std::span{buffer}.subspan(0, n));
    return out;
}

std::string read_line_text(ripper::io::core::reader& reader, const std::size_t capacity)
{
    std::vector<std::byte> buffer(capacity);
    const std::size_t n = reader.read_line(buffer);
    return test_fixture::to_string(std::span{buffer}.subspan(0, n));
}
} // namespace

TEST_CASE("reader parity: memory and file readers expose identical sequential semantics",
          "[io][parity]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_parity_seq.bin"};
    const std::string content = "alpha\nbeta\rgamma\r\ndelta\nlast";

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes(content));
        writer.flush();
    }

    const std::vector<std::byte> bytes = test_fixture::to_bytes(content);

    ripper::io::core::memory_reader mem{bytes};
    ripper::io::core::file_reader file{tmp.path()};

    REQUIRE(mem.size() == file.size());

    // read_line parity, forcing capacity splits so both backends handle partial
    // lines identically.
    for (;;)
    {
        REQUIRE(mem.eof() == file.eof());
        if (mem.eof())
        {
            break;
        }

        const std::string mem_line = read_line_text(mem, 4);
        const std::string file_line = read_line_text(file, 4);

        REQUIRE(mem_line == file_line);
        REQUIRE(mem.tell() == file.tell());
    }

    // Plain read parity.
    mem.seek(0);
    file.seek(0);
    for (;;)
    {
        std::string mem_text;
        std::string file_text;
        const std::string mem_out = read_text(mem, mem_text, 3);
        const std::string file_out = read_text(file, file_text, 3);

        REQUIRE(mem_out == file_out);
        REQUIRE(mem.tell() == file.tell());

        if (mem_out.empty())
        {
            break;
        }
    }
}

TEST_CASE("reader parity: memory and file readers expose identical random-access semantics",
          "[io][parity]")
{
    test_fixture::scoped_temp_file tmp{"io_ripper_core_parity_ra.bin"};
    const std::string content = "0123456789abcdef";

    {
        ripper::io::core::file_writer writer{tmp.path()};
        writer.write(test_fixture::to_bytes(content));
        writer.flush();
    }

    const std::vector<std::byte> bytes = test_fixture::to_bytes(content);

    ripper::io::core::memory_reader mem{bytes};
    ripper::io::core::file_reader file{tmp.path()};

    // read_at parity at and beyond the end of source.
    for (std::size_t offset = 0; offset <= content.size() + 4; ++offset)
    {
        std::array<std::byte, 5> mem_buf{};
        std::array<std::byte, 5> file_buf{};

        const std::size_t mem_n = mem.read_at(mem_buf, offset);
        const std::size_t file_n = file.read_at(file_buf, offset);

        REQUIRE(mem_n == file_n);
        REQUIRE(mem_buf == file_buf);
        REQUIRE(mem.tell() == 0); // read_at must not move the cursor
        REQUIRE(file.tell() == 0);
    }

    // seek/skip/eof clamping parity.
    const std::vector<std::size_t> offsets{0, 1, 5, 16, 100};
    for (const std::size_t offset : offsets)
    {
        mem.seek(offset);
        file.seek(offset);
        REQUIRE(mem.tell() == file.tell());
        REQUIRE(mem.eof() == file.eof());
    }

    mem.seek(0);
    file.seek(0);
    mem.skip(20);
    file.skip(20);
    REQUIRE(mem.tell() == file.tell());
    REQUIRE(mem.eof() == file.eof());
}