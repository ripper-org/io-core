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
/// - `read` transfers bytes from current cursor and advances it.
/// - `read_at` transfers bytes from an absolute offset and may update cursor
///   according to implementation-defined semantics documented by concrete types.
/// - Empty destination buffers are valid and should return 0 without side
///   effects.
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

    /// Return `true` when the input stream reached end-of-file.
    ///
    /// EOF semantics are backend-specific and may become true only after a read
    /// attempt that consumes all available bytes.
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
    /// - Returned value is in `[0, size()]` when bounded by a finite source.
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
    ///         unavailable at current state/position.
    [[nodiscard]] virtual std::byte peek() = 0;

    /// Read up to `buffer.size()` bytes from the current position.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Cursor advances by returned byte count.
    /// - Returns 0 for `buffer.empty()`.
    /// - Returned count is in `[0, buffer.size()]`.
    ///
    /// @throws implementation-defined exception on read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] virtual std::size_t read(std::span<std::byte> buffer) = 0;

    /// Read up to `buffer.size()` bytes starting at absolute `offset`.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Returns 0 for `buffer.empty()`.
    /// - Returned count is in `[0, buffer.size()]`.
    /// - Final cursor position is implementation-defined and must be documented
    ///   by concrete implementations.
    ///
    /// @throws implementation-defined exception on seek/read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] virtual std::size_t read_at(std::span<std::byte> buffer,
                                              const std::uint64_t offset) = 0;

    /// Read a line into `buffer` from the current position.
    ///
    /// Reads until line delimiter, end-of-stream, or destination capacity.
    ///
    /// Preconditions:
    /// - Reader is in a valid readable state.
    ///
    /// Postconditions:
    /// - Cursor advances by consumed source bytes (implementation-defined exact
    ///   delimiter handling).
    /// - Returns 0 for `buffer.empty()`.
    ///
    /// @throws implementation-defined exception on read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] virtual std::size_t read_line(std::span<std::byte> buffer) = 0;

    /// Move the current position to absolute `offset`.
    ///
    /// Preconditions:
    /// - Reader is in a valid seekable state.
    ///
    /// Postconditions:
    /// - Next sequential read starts from implementation-defined position based
    ///   on `offset` and backend constraints.
    ///
    /// @throws implementation-defined exception on invalid offset/state.
    virtual void seek(std::uint64_t offset) = 0;

    /// Advance the current position by `n` bytes.
    ///
    /// Preconditions:
    /// - Reader is in a valid seekable state.
    ///
    /// Postconditions:
    /// - Cursor advances by implementation-defined effective distance.
    ///
    /// @throws implementation-defined exception on invalid state.
    virtual void skip(std::size_t n) = 0;
};
} // namespace ripper::io::core
