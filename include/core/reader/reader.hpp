#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace ripper::io::core
{
    /// Abstract byte-oriented input interface.
    ///
    /// A `reader` provides sequential and random-access byte retrieval from a
    /// backing storage (file, memory, network, etc.).
    ///
    /// Implementations encapsulate storage-specific concerns such as stream
    /// state, buffering, and ownership.
    class reader
    {
    public:
        virtual ~reader() = default;

        /// Return `true` if this reader is open and ready for input.
        [[nodiscard]] virtual bool is_open() = 0;

        /// Return `true` when the input stream reached end-of-file.
        [[nodiscard]] virtual bool eof() = 0;

        /// Return the total size in bytes of the backing data source.
        [[nodiscard]] virtual std::uint64_t size() = 0;

        /// Return the current logical read position.
        [[nodiscard]] virtual std::size_t tell() = 0;

        /// Return the next byte without advancing the current position.
        [[nodiscard]] virtual std::byte peek() = 0;

        /// Read up to `buffer.size()` bytes from the current position.
        ///
        /// Returns the number of bytes actually read.
        [[nodiscard]] virtual std::size_t read(std::span<std::byte> buffer) = 0;

        /// Read up to `buffer.size()` bytes starting at absolute `offset`.
        ///
        /// Returns the number of bytes actually read.
        [[nodiscard]] virtual std::size_t read_at(std::span<std::byte> buffer, const std::uint64_t offset) = 0;

        /// Read a line into `buffer` from the current position.
        ///
        /// Returns the number of bytes transferred into `buffer`.
        [[nodiscard]] virtual std::size_t read_line(std::span<std::byte> buffer) = 0;

        /// Move the current position to absolute `offset`.
        virtual void seek(std::uint64_t offset) = 0;

        /// Advance the current position by `n` bytes.
        virtual void skip(std::size_t n) = 0;
    };
}
