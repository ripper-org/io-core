#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>

#include "core/writer/writer.hpp"

namespace ripper::io::core
{
    /// File-backed `writer` implementation.
    ///
    /// A `file_writer` owns an output file stream and provides byte-wise write,
    /// random seek, flush, and close operations.
    ///
    /// The file is opened in binary mode and truncated on construction.
    /// This class is move-enabled and non-copyable.
    class file_writer : public writer
    {
    public:
        /// Create and open a file writer for `path`.
        ///
        /// The target file is opened in binary mode and truncated if it exists.
        ///
        /// @throws std::ios_base::failure from the stream on I/O failures.
        explicit file_writer(std::filesystem::path path);

        ~file_writer() override = default;

        file_writer(const file_writer &) = delete;
        file_writer &operator=(const file_writer &) = delete;

        file_writer(file_writer &&) noexcept = default;
        file_writer &operator=(file_writer &&) noexcept = default;

        /// Return `true` if the backing file stream is open.
        [[nodiscard]] bool is_open() override;

        /// Return the current logical stream offset.
        [[nodiscard]] std::size_t tell() override;

        /// Write bytes from `buffer` at the current stream offset.
        ///
        /// Returns the number of bytes written.
        ///
        /// @throws std::runtime_error when the writer is closed.
        /// @throws std::ios_base::failure on stream write failures.
        [[nodiscard]] std::size_t write(std::span<const std::byte> buffer) override;

        /// Seek to an absolute stream offset.
        void seek(std::uint64_t offset) override;

        /// Flush buffered data to disk.
        void flush() override;

        /// Close the stream. Calling this on an already-closed stream is safe.
        void close() override;

        /// Return the canonical absolute path for the backing file.
        [[nodiscard]] std::string_view get_path() const;

    private:
        std::filesystem::path _path;
        std::string _canonicalPath;
        std::ofstream _handle;
    };
}
