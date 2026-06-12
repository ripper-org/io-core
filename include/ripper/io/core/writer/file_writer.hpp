#pragma once

#include "ripper/io/core/writer/writer.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>

namespace ripper::io::core
{
/// File-backed `writer` implementation.
///
/// A `file_writer` owns an output file stream and provides byte-wise write,
/// random seek, flush, and close operations.
///
/// The file is opened in binary mode and truncated on construction.
///
/// Ownership and lifetime:
/// - `file_writer` owns the underlying file handle.
/// - Data passed to `write` is copied into the stream immediately.
/// - After move, the moved-from instance is not usable for output operations.
///
/// This class is move-enabled and non-copyable.
class IO_RIPPER_CORE_API file_writer : public writer
{
public:
    /// Create and open a file writer for `path`.
    ///
    /// The target file is opened in binary mode and truncated if it exists.
    ///
    /// Preconditions:
    /// - Parent directory exists and is writable.
    ///
    /// Postconditions (on success):
    /// - `is_open() == true`.
    /// - Existing file contents are replaced.
    ///
    /// @throws std::ios_base::failure from the stream on I/O failures.
    explicit file_writer(std::filesystem::path path);

    ~file_writer() override = default;

    file_writer(const file_writer&) = delete;
    file_writer& operator=(const file_writer&) = delete;

    file_writer(file_writer&&) noexcept = default;
    file_writer& operator=(file_writer&&) noexcept = default;

    /// Return `true` if the backing file stream is open.
    ///
    /// This can be `false` after `close()` or for moved-from instances.
    [[nodiscard]] bool is_open() override;

    /// Return the current logical stream offset.
    ///
    /// @throws std::runtime_error if the stream position cannot be queried.
    [[nodiscard]] std::size_t tell() override;

    /// Write bytes from `buffer` at the current stream offset.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions (on success):
    /// - Stream position advances by returned byte count.
    /// - Returns 0 when `buffer.empty()`.
    ///
    /// @throws std::runtime_error when the writer is closed.
    /// @throws std::ios_base::failure on stream write failures.
    ///
    /// @return Number of bytes written to the stream.
    [[nodiscard]] std::size_t write(std::span<const std::byte> buffer) override;

    /// Seek to an absolute stream offset.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Next `write` starts at `offset`.
    ///
    /// @throws std::runtime_error when the writer is closed.
    /// @throws std::ios_base::failure on seek failures.
    void seek(std::uint64_t offset) override;

    /// Flush buffered data to disk.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Buffered bytes are synchronized to underlying storage.
    ///
    /// @throws std::runtime_error when the writer is closed.
    /// @throws std::ios_base::failure on flush failures.
    void flush() override;

    /// Close the stream. Calling this on an already-closed stream is safe.
    ///
    /// Postconditions:
    /// - `is_open() == false`.
    /// - Subsequent `write`, `seek`, or `flush` calls throw.
    void close() override;

    /// Return the canonical absolute path for the backing file.
    ///
    /// Returned view is valid for the lifetime of this object.
    [[nodiscard]] std::string_view get_path() const;

private:
    std::filesystem::path _path;
    std::string _canonicalPath;
    std::ofstream _handle;
};
} // namespace ripper::io::core
