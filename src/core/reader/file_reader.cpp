#include "core/reader/file_reader.hpp"

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
        : _path{std::move(path)},
          _canonicalPath{std::filesystem::canonical(_path).string()},
          _handle{}
    {
        _handle.exceptions(std::ios::badbit);
        _handle.open(_path, std::ios::binary);
    }

    bool file_reader::is_open()
    {
        return _handle.is_open();
    }

    bool file_reader::eof()
    {
        return _handle.eof();
    }

    std::uint64_t file_reader::size()
    {
        return std::filesystem::file_size(_path);
    }

    std::string_view file_reader::get_path() const
    {
        return _canonicalPath;
    }

    std::size_t file_reader::tell()
    {
        const std::streampos currentPos = _handle.tellg();

        if (currentPos < 0)
        {
            throw std::runtime_error{"Unable to read current stream position for: " + _path.string()};
        }

        return static_cast<std::size_t>(currentPos);
    }

    std::byte file_reader::peek()
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot peek from a closed file_reader: " + _path.string()};
        }

        const std::streampos currentPos = _handle.tellg();

        if (currentPos < 0)
        {
            throw std::runtime_error{"Unable to peek at current stream position for: " + _path.string()};
        }

        char ch = '\0';

        if (!_handle.get(ch))
        {
            throw std::runtime_error{"Failed to read byte while peeking file: " + _path.string()};
        }

        _handle.seekg(currentPos);

        return std::byte{static_cast<unsigned char>(ch)};
    }

    std::size_t file_reader::read(std::span<std::byte> buffer)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot read from a closed file_reader: " + _path.string()};
        }

        if (buffer.empty())
        {
            return 0;
        }

        _handle.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));

        return static_cast<std::size_t>(_handle.gcount());
    }

    std::size_t file_reader::read_at(std::span<std::byte> buffer, const std::uint64_t offset)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot read_at from a closed file_reader: " + _path.string()};
        }

        if (buffer.empty())
        {
            return 0;
        }

        _handle.clear();
        _handle.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        _handle.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));

        return static_cast<std::size_t>(_handle.gcount());
    }

    std::size_t file_reader::read_line(std::span<std::byte> buffer)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot read_line from a closed file_reader: " + _path.string()};
        }

        if (buffer.empty())
        {
            return 0;
        }

        _handle.getline(reinterpret_cast<char *>(buffer.data()), buffer.size());
        return static_cast<std::size_t>(_handle.gcount());
    }

    void file_reader::seek(std::uint64_t offset)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot seek a closed file_reader: " + _path.string()};
        }

        _handle.clear();
        _handle.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    }

    void file_reader::skip(std::size_t n)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot skip on a closed file_reader: " + _path.string()};
        }

        _handle.clear();
        _handle.seekg(static_cast<std::streamoff>(n), std::ios::cur);
    }
}
