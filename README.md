# Ripper IO Core

A modular C++23 library providing consistent interfaces for various I/O operations. All Ripper libraries depend on this library for I/O functionality.

## Build

```bash
make build
```

## Run tests

```bash
make test
```

## Install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix /your/install/prefix
```

## Use from another CMake project (installed package)

```cmake
find_package(io_ripper_core REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

## Use with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
	io_ripper_core
	GIT_REPOSITORY https://github.com/ripper-org/io-core
	GIT_TAG main
)

FetchContent_MakeAvailable(io_ripper_core)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

### CMake options

- `IO_RIPPER_CORE_BUILD_SHARED`: `ON` to build shared library, `OFF` for static.
- `IO_RIPPER_CORE_ENABLE_TESTS`: builds test suite (`ON` by default when this is the top-level project, `OFF` when consumed via FetchContent/add_subdirectory).
