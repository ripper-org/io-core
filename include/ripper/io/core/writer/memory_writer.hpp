#pragma once

#include "ripper/io/core/writer/writer.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ripper::io::core
{
/// In-memory `writer` implementation.
///
/// `memory_writer` writes to caller-owned memory and performs no filesystem
/// interaction.
///
/// Ownership and lifetime:
/// - `memory_writer` never owns the backing storage.
/// - The caller must guarantee the provided mutable/resizable byte container
///   (`std::vector<std::byte>`) outlives the writer.
/// - Constructing from temporary vectors is disallowed to prevent dangling.
///
/// Backend behavior:
/// - Vector-backed mode: writes may resize the vector and preserve random
///   access semantics through `seek`.
class IO_RIPPER_CORE_API memory_writer : public writer
{
public:
    /// Create a writer over a growable non-owning byte vector.
    ///
    /// Preconditions:
    /// - `buffer` outlives this writer.
    ///
    /// Postconditions:
    /// - `tell() == 0`.
    /// - Writer operates in vector-backed growable mode.
    explicit memory_writer(std::vector<std::byte>& buffer);

    /// Create a writer over a growable non-owning byte vector pointer.
    ///
    /// Preconditions:
    /// - `buffer != nullptr`.
    /// - `*buffer` outlives this writer.
    ///
    /// Postconditions:
    /// - `tell() == 0`.
    /// - Writer operates in vector-backed growable mode.
    ///
    /// @throws std::invalid_argument when `buffer == nullptr`.
    explicit memory_writer(std::vector<std::byte>* buffer);

    /// Disallow temporaries that would immediately dangle.
    memory_writer(std::vector<std::byte>&&) = delete;

    ~memory_writer() override = default;

    memory_writer(const memory_writer&) = delete;
    memory_writer& operator=(const memory_writer&) = delete;

    memory_writer(memory_writer&&) noexcept = default;
    memory_writer& operator=(memory_writer&&) noexcept = default;

    /// Return whether this writer accepts output operations.
    ///
    /// Returns `false` after `close()`.
    [[nodiscard]] bool is_open() override;

    /// Return current logical write position.
    [[nodiscard]] std::size_t tell() override;

    /// Write `buffer` at current logical position.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Returns 0 when `buffer.empty()`.
    /// - Vector grows as needed so all bytes are written.
    /// - Advances `tell()` by returned byte count.
    ///
    /// @throws std::runtime_error when writer is closed.
    /// @throws std::bad_alloc on allocation failure in vector-backed mode.
    ///
    /// @return Number of bytes written.
    [[nodiscard]] std::size_t write(std::span<const std::byte> buffer) override;

    /// Set logical write position to `offset`.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Position becomes exactly `offset`.
    ///
    /// @throws std::runtime_error when writer is closed.
    void seek(std::uint64_t offset) override;

    /// Flush pending writes.
    ///
    /// For in-memory storage this is a semantic no-op, but closed-state checks
    /// are still enforced for API consistency with file-backed writers.
    ///
    /// @throws std::runtime_error when writer is closed.
    void flush() override;

    /// Close writer and reject future write/seek/flush operations.
    ///
    /// Postconditions:
    /// - `is_open() == false`.
    /// - Calling `close()` again is safe.
    void close() override;

private:
    std::vector<std::byte>* _buffer_ptr = nullptr;
    std::size_t _position = 0;
    bool _is_open = true;
};
} // namespace ripper::io::core
