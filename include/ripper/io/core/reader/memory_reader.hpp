#pragma once

#include "ripper/io/core/reader/reader.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace ripper::io::core
{
/// In-memory non-owning `reader` implementation.
///
/// `memory_reader` reads from a caller-owned byte buffer and never touches
/// filesystem resources.
///
/// Ownership and lifetime:
/// - `memory_reader` does not own the backing storage.
/// - The caller must ensure the provided fixed-size byte view remains alive for
///   the entire lifetime of this reader.
/// - The backing bytes are treated as immutable for the lifetime of this
///   reader.
/// - Resizing/rebinding the underlying storage while this reader is alive is
///   invalid.
///
/// Mutability model:
/// - Input is treated as immutable bytes (`std::byte const`).
/// - The reader maintains only a logical cursor (`tell/seek/skip`).
class IO_RIPPER_CORE_API memory_reader : public reader
{
public:
    /// Create a reader over a non-owning byte span.
    ///
    /// Preconditions:
    /// - `buffer.data()` remains valid while this reader is used.
    /// - Backing storage is not resized/rebound while this reader is used.
    ///
    /// Postconditions:
    /// - `tell() == 0`.
    explicit memory_reader(std::span<const std::byte> buffer);

    ~memory_reader() override = default;

    memory_reader(const memory_reader&) = default;
    memory_reader& operator=(const memory_reader&) = default;

    memory_reader(memory_reader&&) noexcept = default;
    memory_reader& operator=(memory_reader&&) noexcept = default;

    /// Return whether the reader is available for reads.
    ///
    /// For `memory_reader`, this is always `true`.
    [[nodiscard]] bool is_open() override;

    /// Return whether current logical position is at or past end-of-buffer.
    [[nodiscard]] bool eof() override;

    /// Return total byte size of the backing memory region.
    ///
    /// @throws std::runtime_error if size conversion overflows target type.
    [[nodiscard]] std::uint64_t size() override;

    /// Return current logical read position.
    [[nodiscard]] std::size_t tell() override;

    /// Return next byte without advancing the logical position.
    ///
    /// Preconditions:
    /// - `eof() == false`.
    ///
    /// Postconditions:
    /// - `tell()` is unchanged.
    ///
    /// @throws std::runtime_error when called at end-of-buffer.
    [[nodiscard]] std::byte peek() override;

    /// Read bytes from current position into `buffer`.
    ///
    /// Postconditions:
    /// - Returns 0 when `buffer.empty()` or `eof() == true`.
    /// - Advances position by returned byte count.
    /// - May return less than `buffer.size()` near end-of-buffer.
    ///
    /// @return Number of bytes transferred.
    [[nodiscard]] std::size_t read(std::span<std::byte> buffer) override;

    /// Read bytes starting at absolute `offset`.
    ///
    /// Postconditions:
    /// - Equivalent to `seek(offset)` then `read(buffer)`.
    /// - Final position is `min(offset, size()) + bytes_read`.
    /// - Returns 0 when `buffer.empty()`.
    ///
    /// @return Number of bytes transferred.
    [[nodiscard]] std::size_t read_at(std::span<std::byte> buffer, std::uint64_t offset) override;

    /// Read a line into `buffer`, excluding the newline byte.
    ///
    /// Reads until newline (`'\n'`), end-of-buffer, or `buffer` capacity.
    /// If a newline is encountered it is consumed from the stream position but
    /// is not copied to `buffer`.
    ///
    /// Postconditions:
    /// - Returns 0 when `buffer.empty()` or `eof() == true`.
    /// - Advances `tell()` by consumed input bytes (including delimiter when
    ///   present).
    ///
    /// @return Number of non-delimiter bytes written to `buffer`.
    [[nodiscard]] std::size_t read_line(std::span<std::byte> buffer) override;

    /// Move logical position to `offset`.
    ///
    /// Postconditions:
    /// - Position is clamped to `[0, size()]`.
    /// - Seeking beyond end moves position to `size()`.
    void seek(std::uint64_t offset) override;

    /// Advance logical position by `n` bytes.
    ///
    /// Postconditions:
    /// - Position is clamped to `size()`.
    void skip(std::size_t n) override;

private:
    std::span<const std::byte> _buffer_view;
    std::size_t _position = 0;
};
} // namespace ripper::io::core
