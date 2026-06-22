# Using io-core

## Building and installing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix /your/install/prefix
```

## Integrating with CMake

### Installed package

```cmake
find_package(io_ripper_core REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

### FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    io_ripper_core
    GIT_REPOSITORY https://github.com/ripper-org/io-core
    GIT_TAG        main
)

FetchContent_MakeAvailable(io_ripper_core)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE io_ripper_core::io_ripper_core)
```

## CMake options

| Option                        | Default | Description                             |
| ----------------------------- | ------- | --------------------------------------- |
| `IO_RIPPER_CORE_BUILD_SHARED` | `OFF`   | Build as a shared library (static by default) |
| `IO_RIPPER_CORE_ENABLE_TESTS` | auto    | Enable test suite (`ON` standalone, `OFF` via FetchContent) |

## API overview

io-core provides abstract reader/writer backends:

- **FileReader / FileWriter** — I/O through the local filesystem
- **MemoryReader / MemoryWriter** — I/O over in-memory buffers

All backends implement a common interface, allowing code to operate on files and
memory interchangeably.
