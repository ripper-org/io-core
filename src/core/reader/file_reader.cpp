#include "ripper/io/core/reader/file_reader.hpp"

#include "core/util/numeric_cast.hpp"

#include <cstddef>
#include <filesystem>
#include <ios>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace ripper::io::core
{
file_reader::file_reader(const std::filesystem::path path)
    : _path{std::move(path)}, _canonicalPath{std::filesystem::canonical(_path).string()}, _handle{},
      _size{std::filesystem::file_size(_path)}, _position{0}
{
    _handle.exceptions(std::ios::badbit);
    _handle.open(_path, std::ios::binary);
}

void file_reader::sync_to_position()
{
    _handle.clear();
    _handle.seekg(utils::checked_narrow<std::streamoff>(_position, "seek offset"), std::ios::beg);
}

bool file_reader::is_open()
{
    return _handle.is_open();
}

bool file_reader::eof()
{
    return _position >= _size;
}

std::uint64_t file_reader::size()
{
    return _size;
}

std::string_view file_reader::get_path() const
{
    return _canonicalPath;
}

std::size_t file_reader::tell()
{
    return utils::checked_narrow<std::size_t>(_position, "tell position");
}

std::byte file_reader::peek()
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot peek from a closed file_reader: " + _path.string()};
    }

    if (eof())
    {
        throw std::runtime_error{"Cannot peek at EOF on file_reader: " + _path.string()};
    }

    sync_to_position();

    char ch = '\0';

    if (!_handle.get(ch))
    {
        throw std::runtime_error{"Failed to read byte while peeking file: " + _path.string()};
    }

    return std::byte{static_cast<unsigned char>(ch)};
}

std::size_t file_reader::read(std::span<std::byte> buffer)
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot read from a closed file_reader: " + _path.string()};
    }

    if (buffer.empty() || eof())
    {
        return 0;
    }

    const std::uint64_t remaining = _size - _position;
    const std::uint64_t to_read = std::min<std::uint64_t>(buffer.size(), remaining);

    sync_to_position();

    // std::istream accepts char buffers; conversion from std::byte storage is required here.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    _handle.read(reinterpret_cast<char*>(buffer.data()),
                 utils::checked_narrow<std::streamsize>(to_read, "read size"));

    const std::streamsize got = _handle.gcount();
    if (got > 0)
    {
        _position += static_cast<std::uint64_t>(got);
    }

    return static_cast<std::size_t>(got);
}

std::size_t file_reader::read_at(std::span<std::byte> buffer, const std::uint64_t offset)
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot read_at from a closed file_reader: " + _path.string()};
    }

    if (buffer.empty() || offset >= _size)
    {
        return 0;
    }

    const std::uint64_t remaining = _size - offset;
    const std::uint64_t to_read = std::min<std::uint64_t>(buffer.size(), remaining);

    _handle.clear();
    _handle.seekg(utils::checked_narrow<std::streamoff>(offset, "read_at offset"), std::ios::beg);
    // std::istream accepts char buffers; conversion from std::byte storage is required here.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    _handle.read(reinterpret_cast<char*>(buffer.data()),
                 utils::checked_narrow<std::streamsize>(to_read, "read_at size"));

    const std::streamsize got = _handle.gcount();

    sync_to_position();

    return static_cast<std::size_t>(got);
}

std::size_t file_reader::read_line(std::span<std::byte> buffer)
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot read_line from a closed file_reader: " + _path.string()};
    }

    if (buffer.empty() || eof())
    {
        return 0;
    }

    sync_to_position();

    std::size_t written = 0;
    while (_position < _size && written < buffer.size())
    {
        char ch = '\0';

        if (!_handle.get(ch))
        {
            break;
        }

        ++_position;

        if (ch == '\n')
        {
            break;
        }

        if (ch == '\r')
        {
            if (_handle.peek() == '\n')
            {
                _handle.get(ch);
                ++_position;
            }
            break;
        }

        buffer[written++] = std::byte{static_cast<unsigned char>(ch)};
    }

    return written;
}

void file_reader::seek(std::uint64_t offset)
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot seek a closed file_reader: " + _path.string()};
    }

    _position = std::min(offset, _size);

    sync_to_position();
}

void file_reader::skip(std::size_t n)
{
    if (!is_open())
    {
        throw std::runtime_error{"Cannot skip on a closed file_reader: " + _path.string()};
    }

    const std::uint64_t remaining = _size - _position;
    const std::uint64_t advance = std::min<std::uint64_t>(n, remaining);

    _position += advance;

    sync_to_position();
}
} // namespace ripper::io::core