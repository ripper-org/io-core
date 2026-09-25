# Using io-core

## Building and installing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix /your/install/prefix
```

For a clean release install (no test dependencies such as Catch2), configure
with tests disabled:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DIO_RIPPER_CORE_ENABLE_TESTS=OFF
cmake --build build-release -j
cmake --install build-release --prefix /your/install/prefix
```

You can validate that an installed package works from an external CMake
consumer with:

```bash
make install-check   # installs to a temp prefix, builds/runs a consumer
```

## Integrating with CMake

### Installed package

```cmake
find_package(io_ripper_core CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

### FetchContent

Prefer a released tag for reproducible builds:

```cmake
include(FetchContent)

FetchContent_Declare(
    io_ripper_core
    GIT_REPOSITORY https://github.com/ripper-org/io-core
    GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(io_ripper_core)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

Use `GIT_TAG main` only for development snapshots.

## CMake options

| Option                              | Default | Description                                                 |
| ----------------------------------- | ------- | ----------------------------------------------------------- |
| `IO_RIPPER_CORE_BUILD_SHARED`       | `OFF`   | Build as a shared library (static by default)               |
| `IO_RIPPER_CORE_ENABLE_TESTS`       | auto    | Enable test suite (`ON` standalone, `OFF` via FetchContent) |
| `IO_RIPPER_CORE_TIDY_INCLUDE_TESTS` | `OFF`   | Include tests in clang-tidy analysis                        |

## Platforms and compilers

- CMake 3.20+
- C++23
- Linux: GCC 14+ or Clang 18+
- macOS: Apple Clang 16+
- Windows: MSVC 2022 17.12+ (or Clang with a compatible toolchain)

Continuous integration exercises Debug and Release builds on Linux, macOS, and
Windows, shared-library builds, sanitizer builds, and an installed-package
consumer test.

## API overview

io-core provides abstract reader/writer backends:

- **FileReader / FileWriter** — I/O through the local filesystem
- **MemoryReader / MemoryWriter** — I/O over in-memory buffers

All backends implement a common interface, allowing code to operate on files
and memory interchangeably.

## API contract

`ripper::io::core::reader` and `ripper::io::core::writer` define the public
contract. Concrete backends follow it exactly; the behavior is covered by the
test suite, including backend-parity tests.

- `read()` copies bytes from the current cursor and advances it.
- `read_at()` copies bytes from an absolute offset and **never** moves the
  cursor (analogous to POSIX `pread`).
- `read_line()` copies bytes up to a line delimiter (`\n`, `\r`, or `\r\n`),
  consumes the delimiter, and never writes a NUL terminator. The return value
  is the number of bytes written to the destination. If the destination fills
  before a delimiter is reached, the remainder (including the eventual
  delimiter) is left for subsequent calls.
- `seek()` and `skip()` clamp the cursor to `[0, size()]`.
- `eof()` is true when the cursor is at or past the end of the source.
- Empty destination buffers are valid and return `0` without side effects.
- Writers advance their cursor by the number of bytes written; seeking beyond
  the current end is allowed (sparse gaps are zero-filled in memory backends).
- Instances are **not thread-safe**.
