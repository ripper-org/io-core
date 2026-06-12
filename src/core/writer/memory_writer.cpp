#include "ripper/io/core/writer/memory_writer.hpp"

#include "core/util/numeric_cast.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ripper::io::core
{
memory_writer::memory_writer(std::vector<std::byte>& buffer) : _buffer_ptr{&buffer} {}

memory_writer::memory_writer(std::vector<std::byte>* buffer) : _buffer_ptr{buffer}
{
    if (_buffer_ptr == nullptr)
    {
        throw std::invalid_argument{"memory_writer buffer pointer cannot be null"};
    }
}

bool memory_writer::is_open()
{
    return _is_open;
}

std::size_t memory_writer::tell()
{
    return _position;
}

std::size_t memory_writer::write(std::span<const std::byte> buffer)
{
    if (!_is_open)
    {
        throw std::runtime_error{"Cannot write to a closed memory_writer"};
    }

    if (buffer.empty())
    {
        return 0;
    }

    const std::size_t requested_end = _position + buffer.size();

    if (_buffer_ptr->size() < requested_end)
    {
        _buffer_ptr->resize(requested_end);
    }

    std::ranges::copy(buffer, _buffer_ptr->begin() + utils::checked_narrow<std::ptrdiff_t>(
                                                         _position, "write position"));
    _position = requested_end;

    return buffer.size();
}

void memory_writer::seek(const std::uint64_t offset)
{
    if (!_is_open)
    {
        throw std::runtime_error{"Cannot seek a closed memory_writer"};
    }

    _position = utils::checked_narrow<std::size_t>(offset, "seek offset");
}

void memory_writer::flush()
{
    if (!_is_open)
    {
        throw std::runtime_error{"Cannot flush a closed memory_writer"};
    }
}

void memory_writer::close()
{
    _is_open = false;
}
} // namespace ripper::io::core
