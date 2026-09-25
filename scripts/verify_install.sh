#!/usr/bin/env bash
#
# verify_install.sh
#
# Installs io-core to a temporary prefix and validates that an external CMake
# consumer can find_package() the installed package, link against it, and run.
# Exercises both the static and shared configurations.
#
# Usage:
#   ./scripts/verify_install.sh          # uses Release
#   BUILD_TYPE=Debug ./scripts/verify_install.sh
#
# Exits non-zero on the first failure.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Release}"
WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/io_ripper_core_install_check.XXXXXX")"
trap 'rm -rf "${WORK_DIR}"' EXIT

check_consumer() {
    local variant="$1"
    local build_dir="${WORK_DIR}/${variant}-build"
    local prefix="${WORK_DIR}/${variant}-prefix"
    local consumer_dir="${WORK_DIR}/${variant}-consumer"
    local shared_flag=OFF
    [[ "${variant}" == "shared" ]] && shared_flag=ON

    echo "==> [${variant}] configure, build, install"
    cmake -S "${ROOT_DIR}" -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DIO_RIPPER_CORE_ENABLE_TESTS=OFF \
        -DIO_RIPPER_CORE_BUILD_SHARED="${shared_flag}"
    cmake --build "${build_dir}" -j
    cmake --install "${build_dir}" --prefix "${prefix}"

    echo "==> [${variant}] configure and build external consumer"
    mkdir -p "${consumer_dir}"
    cat > "${consumer_dir}/CMakeLists.txt" <<'CMAKE'
cmake_minimum_required(VERSION 3.20)
project(io_ripper_core_consumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(io_ripper_core CONFIG REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE io_ripper_core::io_ripper_core)
CMAKE
    cat > "${consumer_dir}/main.cpp" <<'CPP'
#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/io/core/writer/memory_writer.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

using namespace ripper::io::core;

static std::vector<std::byte> to_bytes(const std::string& s)
{
    std::vector<std::byte> out(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
        out[i] = std::byte{static_cast<unsigned char>(s[i])};
    return out;
}

int main(int argc, char** argv)
{
    if (argc != 2)
        return 2;

    const std::filesystem::path file{argv[1]};
    const std::string text = "hello from io-core consumer";

    {
        file_writer writer{file};
        writer.write(to_bytes(text));
        writer.flush();
    }

    {
        file_reader reader{file};
        std::vector<std::byte> buffer(static_cast<std::size_t>(reader.size()));
        const std::size_t n = reader.read(buffer);
        if (n != text.size())
            return 3;
        for (std::size_t i = 0; i < n; ++i)
            if (buffer[i] != static_cast<std::byte>(text[i]))
                return 4;
    }

    std::vector<std::byte> memory;
    {
        memory_writer writer{memory};
        writer.write(to_bytes(text));
    }
    {
        memory_reader reader{memory};
        std::vector<std::byte> buffer(static_cast<std::size_t>(reader.size()));
        const std::size_t n = reader.read(buffer);
        if (n != text.size())
            return 5;
    }

    return 0;
}
CPP

    cmake -S "${consumer_dir}" -B "${consumer_dir}/build" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_PREFIX_PATH="${prefix}"
    cmake --build "${consumer_dir}/build" -j

    echo "==> [${variant}] run consumer"
    "${consumer_dir}/build/consumer" "${WORK_DIR}/${variant}-consumer.out"
    rm -f "${WORK_DIR}/${variant}-consumer.out"
}

check_consumer static
check_consumer shared

echo "==> install verification passed (static + shared)"
