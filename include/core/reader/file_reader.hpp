#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>
#include <string_view>

#include "core/reader/reader.hpp"

namespace ripper::io::core
{
    /// File-backed `reader` implementation.
    ///
    /// A `file_reader` owns an input file stream and provides sequential and
    /// random-access byte reads.
    ///
    /// The file is opened in binary mode during construction.
    /// This class is move-enabled and non-copyable.
    class file_reader : public reader
    {
    public:
        /// Create and open a file reader for `path`.
        ///
        /// @throws std::runtime_error if the file cannot be opened.
        explicit file_reader(const std::filesystem::path path);

        ~file_reader() override = default;

        file_reader(const file_reader &) = delete;
        file_reader &operator=(const file_reader &) = delete;

        file_reader(file_reader &&) noexcept = default;
        file_reader &operator=(file_reader &&) noexcept = default;

        /// Return `true` if the backing file stream is open.
        [[nodiscard]] bool is_open() const noexcept override;

        /// Return `true` when the stream reached end-of-file.
        [[nodiscard]] bool eof() const noexcept override;

        /// Return the total size in bytes of the backing file.
        [[nodiscard]] std::uint64_t size() const noexcept override;

        /// Return the current logical stream offset.
        [[nodiscard]] std::size_t tell() const noexcept override;

        /// Peek the next byte without advancing the stream offset.
        [[nodiscard]] std::byte peek() override;

        /// Read bytes from the current stream offset.
        ///
        /// Returns the number of bytes read.
        [[nodiscard]] std::size_t read(std::span<std::byte> buffer) override;

        /// Read bytes starting from absolute `offset`.
        ///
        /// Returns the number of bytes read.
        [[nodiscard]] std::size_t read_at(std::span<std::byte> buffer, const std::uint64_t offset) override;

        /// Read a line into `buffer`.
        ///
        /// Returns the number of bytes transferred.
        [[nodiscard]] std::size_t read_line(std::span<std::byte> buffer) override;

        /// Move stream position to absolute `offset`.
        void seek(std::uint64_t offset) override;

        /// Advance stream position by `n` bytes.
        void skip(std::size_t n) override;

        /// Return the canonical absolute path of the backing file.
        [[nodiscard]] std::string_view get_path() const noexcept;

    private:
        std::filesystem::path _path;
        std::string _canonicalPath;
        std::ifstream _handle;

        std::uint64_t _currentOffset{0};
    };
}
