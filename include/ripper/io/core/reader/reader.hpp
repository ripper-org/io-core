#pragma once

#include "ripper/io/core/io_ripper_core_export.h"

#include <cstddef>
#include <cstdint>
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
///
/// Contract baseline for all implementations:
/// - A reader exposes a single logical cursor (`tell`, `seek`, `skip`).
/// - `read` transfers bytes from the current cursor and advances it.
/// - `read_at` transfers bytes from an absolute offset and never modifies the
///   cursor (random-access semantics, analogous to POSIX `pread`).
/// - `read_line` transfers bytes up to a line delimiter and advances the
///   cursor past the consumed delimiter.
/// - Empty destination buffers are valid and return 0 without side effects.
/// - `tell` always reports a value in `[0, size()]`.
/// - `seek` and `skip` clamp the cursor to `[0, size()]`.
/// - `eof` becomes `true` when the cursor is at or past the end of the source.
///
/// Lifetime and ownership:
/// - Ownership of backing resources is implementation-defined.
/// - Callers must follow lifetime guarantees documented by concrete readers
///   (for example, non-owning memory readers require external buffer lifetime).
///
/// Threading:
/// - Implementations are not required to be thread-safe.
class IO_RIPPER_CORE_API reader
{
public:
    virtual ~reader() = default;

    /// Return `true` if this reader is open and ready for input.
    ///
    /// Semantics are implementation-defined for in-memory backends, but must be
    /// stable and consistent with operation validity checks.
    [[nodiscard]] virtual bool is_open() = 0;

    /// Return `true` when the logical cursor is at or past the end of the
    /// backing data source.
    ///
    /// This is a position-based check and is identical across backends: it is
    /// `true` whenever `tell() == size()`, even if no read attempt has yet been
    /// made beyond the final byte.
    [[nodiscard]] virtual bool eof() = 0;

    /// Return the total size in bytes of the backing data source.
    ///
    /// Should not modify cursor state.
    ///
    /// @throws implementation-defined exception when size cannot be determined.
    [[nodiscard]] virtual std::uint64_t size() = 0;

    /// Return the current logical read position.
    ///
    /// Postconditions:
    /// - Returned value is in `[0, size()]`.
    ///
    /// @throws implementation-defined exception on invalid or unavailable state.
    [[nodiscard]] virtual std::size_t tell() = 0;

    /// Return the next byte without advancing the current position.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions (on success):
    /// - Cursor position is unchanged.
    ///
    /// @throws implementation-defined exception when peeking is invalid or
    ///         unavailable at current state/position (including at end-of-source).
    [[nodiscard]] virtual std::byte peek() = 0;

    /// Read up to `buffer.size()` bytes from the current position.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Cursor advances by returned byte count.
    /// - Returns 0 for `buffer.empty()`.
    /// - Returns 0 when the cursor is already at end-of-source.
    /// - Returned count is in `[0, buffer.size()]`.
    ///
    /// @throws implementation-defined exception on read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] virtual std::size_t read(std::span<std::byte> buffer) = 0;

    /// Read up to `buffer.size()` bytes starting at absolute `offset`.
    ///
    /// This operation never modifies the cursor.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Returns 0 for `buffer.empty()`.
    /// - Returns 0 when `offset` is at or beyond `size()`.
    /// - Returned count is in `[0, buffer.size()]`.
    /// - Cursor position is unchanged.
    ///
    /// @throws implementation-defined exception on seek/read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] virtual std::size_t read_at(std::span<std::byte> buffer,
                                              std::uint64_t offset) = 0;

    /// Read a line into `buffer` from the current position.
    ///
    /// Reads until a line delimiter (`"\n"`, `"\r"`, or `"\r\n"`), end-of-source,
    /// or destination capacity is reached. The delimiter is consumed from the
    /// source but is never written into `buffer`. No null terminator is written.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Cursor advances by consumed source bytes, including a consumed
    ///   delimiter when one is present.
    /// - Returns 0 for `buffer.empty()`.
    /// - Returns 0 when the cursor is already at end-of-source.
    /// - Returned count equals the number of bytes written into `buffer` and
    ///   never includes the delimiter.
    /// - If `buffer` becomes full before a delimiter is reached, reading stops
    ///   there and the remaining bytes (including the eventual delimiter) are
    ///   left for subsequent calls.
    ///
    /// @throws implementation-defined exception on read failures.
    ///
    /// @return Number of bytes written into `buffer`.
    [[nodiscard]] virtual std::size_t read_line(std::span<std::byte> buffer) = 0;

    /// Move the current position to absolute `offset`.
    ///
    /// Preconditions:
    /// - Reader is in a valid seekable state.
    ///
    /// Postconditions:
    /// - The cursor is clamped to `[0, size()]`; seeking beyond the end places
    ///   the cursor at `size()`.
    /// - Next sequential read starts from the resulting cursor position.
    ///
    /// @throws implementation-defined exception on invalid state.
    virtual void seek(std::uint64_t offset) = 0;

    /// Advance the current position by `n` bytes.
    ///
    /// Preconditions:
    /// - Reader is in a valid seekable state.
    ///
    /// Postconditions:
    /// - The cursor is clamped to `[0, size()]`; skipping past the end places
    ///   the cursor at `size()`.
    ///
    /// @throws implementation-defined exception on invalid state.
    virtual void skip(std::size_t n) = 0;
};
} // namespace ripper::io::core
