# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Backend-parity tests asserting that the file and memory readers expose
  identical sequential and random-access semantics.
- Edge-case coverage: empty input, binary data containing NUL bytes, `\n`,
  `\r`, and `\r\n` line endings, empty and over-capacity lines, missing
  trailing newline, cursor clamping, `read_at` cursor immutability, move
  semantics, and write-position overflow.
- `scripts/verify_install.sh`: installs the package to a temporary prefix and
  validates that an external CMake consumer can `find_package()`, link, and
  run against it in both static and shared configurations (`make install-check`).
- CI coverage for Release builds, macOS, shared-library builds, the
  installed-package consumer test, and ASan/UBSan sanitizers.

### Changed

- **API contract finalized.** The behavior of `read`, `read_at`, `read_line`,
  `seek`, `skip`, and `eof` is now identical across all backends and is
  documented in the public headers.
  - `read_at` is now **non-mutating** (cursor stays put, analogous to `pread`).
  - `read_line` returns only the bytes copied into the destination (delimiter
    excluded) and supports `\n`, `\r`, and `\r\n`; no NUL terminator is
    written. A full buffer defers the remainder, including the delimiter, to
    the next call.
  - `eof` is position-based: it is true whenever the cursor is at or past the
    end of the source.
  - `seek`/`skip` clamp the cursor to `[0, size()]`.
- `checked_narrow` rewritten on `std::in_range`, fixing incorrect handling of
  widening conversions (e.g. signed-to-unsigned and signed-to-signed
  widening).
- `memory_writer` move construction/assignment now detach the source: the
  moved-from writer is closed and unusable.
- `memory_writer::write` throws `std::overflow_error` when the target position
  would overflow `std::size_t`.
- Static builds now propagate `IO_RIPPER_CORE_STATIC_DEFINE` to consumers so
  the generated export header is correct on all platforms, including Windows.
- The library is built position-independent so a static archive can be linked
  into shared consumers.
- `file_reader` and `file_writer` report accurate path and cursor behavior and
  perform checked offset conversions.
- Test fixtures now use unique temporary paths and verify fixture creation.

### Fixed

- `file_reader` no longer reports inconsistent `eof()`/`read_line()` results
  caused by leaking `std::ifstream` state.
- Empty-line handling in `read_line` no longer depends on backend-specific
  `gcount()` behavior.