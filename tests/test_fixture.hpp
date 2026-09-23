#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace test_fixture
{
namespace fs = std::filesystem;

inline std::vector<std::byte> to_bytes(const std::string& text)
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

/// Produce a unique temp path so tests never collide with one another or with
/// stale files from previous runs.
inline fs::path unique_temp_path(const std::string& base)
{
    static std::atomic<std::uint64_t> counter{0};
    const std::uint64_t id = counter.fetch_add(1);
    return fs::temp_directory_path() / (base + "_" + std::to_string(id) + ".bin");
}

inline fs::path shared_reader_fixture_path()
{
    return unique_temp_path("io_ripper_core_reader_fixture");
}

inline void ensure_reader_fixture_file(const fs::path& path,
                                       const std::string& payload = "0123456789")
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
    out.close();

    if (!out)
    {
        throw std::runtime_error{"Failed to create reader fixture file: " + path.string()};
    }
}

class scoped_temp_file
{
public:
    explicit scoped_temp_file(std::string base) : path_(unique_temp_path(std::move(base))) {}

    ~scoped_temp_file()
    {
        std::error_code ec;
        fs::remove(path_, ec);
    }

    scoped_temp_file(const scoped_temp_file&) = delete;
    scoped_temp_file& operator=(const scoped_temp_file&) = delete;

    scoped_temp_file(scoped_temp_file&&) noexcept = default;
    scoped_temp_file& operator=(scoped_temp_file&&) noexcept = default;

    const fs::path& path() const
    {
        return path_;
    }

private:
    fs::path path_;
};
} // namespace test_fixture