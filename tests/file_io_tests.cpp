#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "core/reader/file_reader.hpp"
#include "core/writer/file_writer.hpp"

namespace
{
    namespace fs = std::filesystem;

    fs::path make_temp_file_path()
    {
        return fs::temp_directory_path() / fs::path{"io_ripper_core_test.bin"};
    }

    std::vector<std::byte> to_bytes(const std::string &text)
    {
        std::vector<std::byte> out;
        out.reserve(text.size());

        for (const char ch : text)
        {
            out.push_back(static_cast<std::byte>(ch));
        }

        return out;
    }

    std::string to_string(const std::span<const std::byte> bytes)
    {
        std::string out;
        out.reserve(bytes.size());

        for (const std::byte b : bytes)
        {
            out.push_back(static_cast<char>(b));
        }

        return out;
    }
}

TEST_CASE("file_writer writes bytes and file_reader reads them back", "[io][file]")
{
    const fs::path filePath = make_temp_file_path();
    const std::string payload = "ripper-io-core";
    const std::vector<std::byte> payloadBytes = to_bytes(payload);

    {
        ripper::io::core::file_writer writer{filePath};
        REQUIRE(writer.is_open());
        REQUIRE(writer.tell() == 0);

        const std::size_t written = writer.write(payloadBytes);
        REQUIRE(written == payloadBytes.size());
        REQUIRE(writer.tell() == payloadBytes.size());

        writer.flush();
    }

    {
        ripper::io::core::file_reader reader{filePath};
        REQUIRE(reader.is_open());
        REQUIRE(reader.size() == payloadBytes.size());
        REQUIRE(reader.tell() == 0);

        std::vector<std::byte> buffer(payloadBytes.size());
        const std::size_t bytesRead = reader.read(buffer);
        REQUIRE(bytesRead == payloadBytes.size());
        REQUIRE(to_string(buffer) == payload);
    }

    fs::remove(filePath);
}

TEST_CASE("file_reader read_at returns expected window", "[io][file]")
{
    const fs::path filePath = make_temp_file_path();
    const std::string payload = "0123456789";
    const std::vector<std::byte> payloadBytes = to_bytes(payload);

    {
        ripper::io::core::file_writer writer{filePath};
        REQUIRE(writer.write(payloadBytes) == payloadBytes.size());
        writer.flush();
    }

    {
        ripper::io::core::file_reader reader{filePath};

        std::array<std::byte, 4> slice{};
        const std::size_t bytesRead = reader.read_at(slice, 3);

        REQUIRE(bytesRead == slice.size());
        REQUIRE(to_string(slice) == "3456");
    }

    fs::remove(filePath);
}

TEST_CASE("file_writer close prevents further writes", "[io][file]")
{
    const fs::path filePath = make_temp_file_path();

    ripper::io::core::file_writer writer{filePath};
    REQUIRE(writer.is_open());

    writer.close();
    REQUIRE_FALSE(writer.is_open());

    const std::vector<std::byte> data = to_bytes("abc");
    REQUIRE_THROWS(writer.write(data));

    fs::remove(filePath);
}
