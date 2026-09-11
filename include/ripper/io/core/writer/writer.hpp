#pragma once

#include "ripper/io/core/io_ripper_core_export.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace ripper::io::core
{
/// Abstract byte-oriented output interface.
///
/// A `writer` provides sequential and random-access byte emission to a backing
/// storage (file, buffer, network, etc.). It is the output-side counterpart
/// of `reader` and is intended for generic I/O serialization workflows.
///
/// Implementations encapsulate storage-specific concerns such as buffering,
/// stream state, and resource ownership.
///
/// Contract baseline for all implementations:
/// - A writer exposes a single logical cursor (`tell`, `seek`).
/// - `write` emits bytes at the current cursor and advances by `buffer.size()`.
/// - Empty source buffers are valid no-ops.
/// - `close` transitions the writer to a closed state; operation validity after
///   close is implementation-defined and must be documented by concrete types.
///
/// Lifetime and ownership:
/// - Ownership of backing resources is implementation-defined.
/// - Callers must follow lifetime guarantees documented by concrete writers
///   (for example, non-owning memory writers require external buffer lifetime).
///
/// Threading:
/// - Implementations are not required to be thread-safe.
class IO_RIPPER_CORE_API writer
{
public:
    virtual ~writer() = default;

    /// Return `true` if this writer is open and ready for output.
    ///
    /// Must be consistent with whether mutating operations are accepted.
    [[nodiscard]] virtual bool is_open() = 0;

    /// Return the current stream write position.
    ///
    /// @throws implementation-defined exception on invalid or unavailable state.
    [[nodiscard]] virtual std::size_t tell() = 0;

    /// Write all bytes from `buffer` at the current stream position.
    ///
    /// Preconditions:
    /// - Writer is in a valid writable state.
    ///
    /// Postconditions:
    /// - Cursor advances by `buffer.size()`.
    /// - `buffer.empty()` is a valid no-op.
    /// - Either all bytes are written, or the call fails.
    ///
    /// Implementations are responsible for handling backend constraints such as
    /// partial writes or blocking internally; callers must not observe them.
    ///
    /// @throws implementation-defined exception on write failures.
    virtual void write(std::span<const std::byte> buffer) = 0;

    /// Move the write position to `offset` from stream start.
    ///
    /// Preconditions:
    /// - Writer is in a valid seekable state.
    ///
    /// Postconditions:
    /// - Next write starts from implementation-defined position based on
    ///   backend and `offset`.
    ///
    /// @throws implementation-defined exception on invalid offset/state.
    virtual void seek(std::uint64_t offset) = 0;

    /// Flush buffered output to backing storage.
    ///
    /// Implementations without buffering may treat this as a no-op.
    ///
    /// @throws implementation-defined exception on flush failures.
    virtual void flush() = 0;

    /// Close the stream and release owned resources.
    ///
    /// Postconditions:
    /// - Writer transitions to closed state.
    /// - `is_open()` reflects the closed state.
    ///
    /// Calling `close` multiple times should be safe unless explicitly
    /// documented otherwise by a concrete implementation.
    virtual void close() = 0;
};
} // namespace ripper::io::core
