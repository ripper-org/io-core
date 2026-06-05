#include "core/writer/file_writer.hpp"

#include <cstddef>
#include <filesystem>
#include <ios>
#include <span>
#include <stdexcept>
#include <utility>

namespace ripper::io::core
{
    /// Construct a file-backed writer and open the target file.
    ///
    /// The target is opened in binary output mode with truncation.
    /// Throws if the file cannot be opened.
    file_writer::file_writer(std::filesystem::path path)
        : _path{std::move(path)},
          _canonicalPath{std::filesystem::absolute(_path).string()},
          _handle{}
    {
        _handle.exceptions(std::ios::badbit | std::ios::failbit);
        _handle.open(_path, std::ios::binary | std::ios::out | std::ios::trunc);
    }

    /// Return whether the underlying output stream is open.
    bool file_writer::is_open()
    {
        return _handle.is_open();
    }

    /// Return the current logical write offset.
    std::size_t file_writer::tell()
    {
        const std::streampos currentPos = _handle.tellp();

        if (currentPos < 0)
        {
            throw std::runtime_error{"Unable to read current write position for: " + _path.string()};
        }

        return static_cast<std::size_t>(currentPos);
    }

    /// Return the canonical absolute file path.
    std::string_view file_writer::get_path() const
    {
        return _canonicalPath;
    }

    /// Write bytes from `buffer` to the current stream position.
    ///
    /// Returns 0 only for an empty input buffer.
    /// Throws when the stream is closed or the underlying write fails.
    std::size_t file_writer::write(std::span<const std::byte> buffer)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot write to a closed file_writer: " + _path.string()};
        }

        if (buffer.empty())
        {
            return 0;
        }

        _handle.write(reinterpret_cast<const char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));

        return buffer.size();
    }

    /// Set the write position to `offset` from stream start.
    void file_writer::seek(std::uint64_t offset)
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot seek a closed file_writer: " + _path.string()};
        }

        _handle.clear();
        _handle.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    }

    /// Flush buffered bytes to backing storage.
    void file_writer::flush()
    {
        if (!is_open())
        {
            throw std::runtime_error{"Cannot flush a closed file_writer: " + _path.string()};
        }

        _handle.flush();
    }

    /// Close the stream and release file resources.
    void file_writer::close()
    {
        if (!is_open())
        {
            return;
        }

        _handle.flush();
        _handle.close();
    }
}
