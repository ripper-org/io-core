#pragma once

#include "ripper/io/core/reader/reader.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>

namespace ripper::io::core
{
/// File-backed `reader` implementation.
///
/// A `file_reader` owns an input file stream and provides sequential and
/// random-access byte reads.
///
/// The file is opened in binary mode during construction and remains owned by
/// this object until destruction or move.
///
/// Ownership and lifetime:
/// - `file_reader` owns the underlying file handle.
/// - Any data returned through read operations is copied into caller-provided
///   buffers; no borrowed view into internal storage is exposed.
/// - After move, the moved-from instance must be treated as not usable for I/O.
///
/// This class is move-enabled and non-copyable.
class IO_RIPPER_CORE_API file_reader : public reader
{
public:
    /// Create and open a file reader for `path`.
    ///
    /// Preconditions:
    /// - `path` refers to an existing readable file.
    ///
    /// Postconditions (on success):
    /// - `is_open() == true`.
    /// - `get_path()` returns the canonical absolute path.
    ///
    /// @throws std::filesystem::filesystem_error during path canonicalization.
    /// @throws std::ios_base::failure from the stream on I/O failures.
    explicit file_reader(const std::filesystem::path path);

    ~file_reader() override = default;

    file_reader(const file_reader&) = delete;
    file_reader& operator=(const file_reader&) = delete;

    file_reader(file_reader&&) noexcept = default;
    file_reader& operator=(file_reader&&) noexcept = default;

    /// Return `true` if the backing file stream is open.
    ///
    /// This can be `false` for moved-from instances.
    [[nodiscard]] bool is_open() override;

    /// Return `true` when the stream reached end-of-file.
    ///
    /// EOF is stateful and may become true only after a read attempt beyond the
    /// available bytes.
    [[nodiscard]] bool eof() override;

    /// Return the total size in bytes of the backing file.
    ///
    /// @throws std::filesystem::filesystem_error if file size cannot be queried.
    [[nodiscard]] std::uint64_t size() override;

    /// Return the current logical stream offset.
    ///
    /// @throws std::runtime_error if the stream position cannot be queried.
    [[nodiscard]] std::size_t tell() override;

    /// Peek the next byte without advancing the stream offset.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    /// - Current position is readable.
    ///
    /// Postconditions (on success):
    /// - Stream position is unchanged.
    ///
    /// @throws std::runtime_error if stream is closed or byte retrieval fails.
    [[nodiscard]] std::byte peek() override;

    /// Read bytes from the current stream offset.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Advances the stream by the returned byte count.
    /// - Returns 0 when `buffer.empty()`.
    /// - May return less than `buffer.size()` at end-of-file.
    ///
    /// @throws std::runtime_error if stream is closed.
    /// @throws std::ios_base::failure on unrecoverable stream I/O failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] std::size_t read(std::span<std::byte> buffer) override;

    /// Read bytes starting from absolute `offset`.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Stream position is set to `offset` before reading.
    /// - Final position is `offset + bytes_read`.
    /// - Returns 0 when `buffer.empty()`.
    ///
    /// @throws std::runtime_error if stream is closed.
    /// @throws std::ios_base::failure on seek/read failures.
    ///
    /// @return Number of bytes transferred into `buffer`.
    [[nodiscard]] std::size_t read_at(std::span<std::byte> buffer,
                                      const std::uint64_t offset) override;

    /// Read a line into `buffer`.
    ///
    /// Reads until newline, stream EOF, or `buffer` capacity is reached.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Advances stream position past consumed input.
    /// - Returns 0 when `buffer.empty()`.
    ///
    /// @throws std::runtime_error if stream is closed.
    /// @throws std::ios_base::failure on stream failures.
    ///
    /// @return Number of bytes reported by the stream extraction call.
    [[nodiscard]] std::size_t read_line(std::span<std::byte> buffer) override;

    /// Move stream position to absolute `offset`.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Next read operation starts from `offset`.
    ///
    /// @throws std::runtime_error if stream is closed.
    /// @throws std::ios_base::failure on seek failures.
    void seek(std::uint64_t offset) override;

    /// Advance stream position by `n` bytes.
    ///
    /// Preconditions:
    /// - `is_open() == true`.
    ///
    /// Postconditions:
    /// - Position advances by `n` from current location.
    ///
    /// @throws std::runtime_error if stream is closed.
    /// @throws std::ios_base::failure on seek failures.
    void skip(std::size_t n) override;

    /// Return the canonical absolute path of the backing file.
    ///
    /// Returned view is valid for the lifetime of this object.
    [[nodiscard]] std::string_view get_path() const;

private:
    std::filesystem::path _path;
    std::string _canonicalPath;
    std::ifstream _handle;
};
} // namespace ripper::io::core
