#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

namespace test_fixture
{
    namespace fs = std::filesystem;

    inline std::vector<std::byte> to_bytes(const std::string &text)
    {
        std::vector<std::byte> out;
        out.reserve(text.size());

        for (const char ch : text)
        {
            out.push_back(static_cast<std::byte>(ch));
        }

        return out;
    }

    inline std::string to_string(const std::span<const std::byte> bytes)
    {
        std::string out;
        out.reserve(bytes.size());

        for (const std::byte b : bytes)
        {
            out.push_back(static_cast<char>(b));
        }

        return out;
    }

    inline fs::path shared_reader_fixture_path()
    {
        return fs::temp_directory_path() / fs::path{"io_ripper_core_reader_fixture.bin"};
    }

    inline void ensure_reader_fixture_file(const std::string &payload = "0123456789")
    {
        const fs::path path = shared_reader_fixture_path();

        if (fs::exists(path))
        {
            return;
        }

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    }

    class scoped_temp_file
    {
    public:
        explicit scoped_temp_file(std::string filename)
            : path_(fs::temp_directory_path() / std::move(filename))
        {
            fs::remove(path_);
        }

        ~scoped_temp_file()
        {
            fs::remove(path_);
        }

        const fs::path &path() const
        {
            return path_;
        }

    private:
        fs::path path_;
    };
}
