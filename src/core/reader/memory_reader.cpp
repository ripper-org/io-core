#include "ripper/io/core/reader/memory_reader.hpp"

#include "core/util/numeric_cast.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace ripper::io::core
{
memory_reader::memory_reader(std::span<const std::byte> buffer) : _buffer_view{buffer} {}

bool memory_reader::is_open()
{
    return true;
}

bool memory_reader::eof()
{
    return _position >= _buffer_view.size();
}

std::uint64_t memory_reader::size()
{
    return utils::checked_narrow<std::uint64_t>(_buffer_view.size(), "buffer size");
}

std::size_t memory_reader::tell()
{
    return _position;
}

std::byte memory_reader::peek()
{
    if (eof())
    {
        throw std::runtime_error{"Cannot peek at EOF on memory_reader"};
    }

    return _buffer_view[_position];
}

std::size_t memory_reader::read(std::span<std::byte> buffer)
{
    if (buffer.empty() || eof())
    {
        return 0;
    }

    const std::size_t remaining = _buffer_view.size() - _position;
    const std::size_t bytes_to_read = std::min(buffer.size(), remaining);

    const auto source = _buffer_view.subspan(_position, bytes_to_read);

    std::ranges::copy(source, buffer.begin());

    _position += bytes_to_read;

    return bytes_to_read;
}

std::size_t memory_reader::read_at(std::span<std::byte> buffer, const std::uint64_t offset)
{
    if (buffer.empty())
    {
        return 0;
    }

    seek(offset);
    return read(buffer);
}

std::size_t memory_reader::read_line(std::span<std::byte> buffer)
{
    if (buffer.empty() || eof())
    {
        return 0;
    }

    std::size_t written = 0;
    while (_position < _buffer_view.size() && written < buffer.size())
    {
        const std::byte current = _buffer_view[_position++];

        if (current == std::byte{'\n'})
        {
            break;
        }

        buffer[written++] = current;
    }

    return written;
}

void memory_reader::seek(const std::uint64_t offset)
{
    const std::size_t requested = utils::checked_narrow<std::size_t>(offset, "seek offset");

    _position = std::min(requested, _buffer_view.size());
}

void memory_reader::skip(const std::size_t n)
{
    const std::size_t remaining = _buffer_view.size() - _position;

    _position += std::min(n, remaining);
}
} // namespace ripper::io::core
