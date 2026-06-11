cmake_minimum_required(VERSION 3.20)

project(io_ripper_core
    VERSION 0.1.0
    LANGUAGES CXX
)

option(IO_RIPPER_CORE_BUILD_SHARED "Build io_ripper_core as a shared library" ON)

# Keep tests off by default when consumed via add_subdirectory/FetchContent.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(IO_RIPPER_CORE_ENABLE_TESTS "Build io_ripper_core tests" ON)
else()
    option(IO_RIPPER_CORE_ENABLE_TESTS "Build io_ripper_core tests" OFF)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(IO_RIPPER_CORE_ENABLE_TESTS)
    include(CTest)
endif()

file(GLOB_RECURSE IO_RIPPER_CORE_SOURCES
    CONFIGURE_DEPENDS
    src/core/*.cpp
)

file(GLOB_RECURSE IO_RIPPER_CORE_HEADERS
    CONFIGURE_DEPENDS
    include/core/*.hpp
)

file(GLOB_RECURSE IO_RIPPER_CORE_TEST_FILES
    CONFIGURE_DEPENDS
    tests/*.cpp
    tests/*.hpp
)

set(IO_RIPPER_CORE_FORMAT_FILES
    ${IO_RIPPER_CORE_HEADERS}
    ${IO_RIPPER_CORE_SOURCES}
    ${IO_RIPPER_CORE_TEST_FILES}
)

set(IO_RIPPER_CORE_TIDY_FILES ${IO_RIPPER_CORE_SOURCES})
if(IO_RIPPER_CORE_ENABLE_TESTS)
    list(APPEND IO_RIPPER_CORE_TIDY_FILES ${IO_RIPPER_CORE_TEST_FILES})
endif()

find_program(IO_RIPPER_CORE_CLANG_FORMAT_BIN NAMES clang-format)
if(IO_RIPPER_CORE_CLANG_FORMAT_BIN)
    add_custom_target(format
        COMMAND ${IO_RIPPER_CORE_CLANG_FORMAT_BIN} -i ${IO_RIPPER_CORE_FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source files with clang-format"
        VERBATIM
    )

    add_custom_target(format-check
        COMMAND ${IO_RIPPER_CORE_CLANG_FORMAT_BIN} --dry-run --Werror ${IO_RIPPER_CORE_FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking source formatting with clang-format"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found; 'format' and 'format-check' targets will fail until installed.")

    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )

    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

find_program(IO_RIPPER_CORE_CLANG_TIDY_BIN NAMES clang-tidy)
if(IO_RIPPER_CORE_CLANG_TIDY_BIN)
    add_custom_target(tidy
        COMMAND ${IO_RIPPER_CORE_CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR} ${IO_RIPPER_CORE_TIDY_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy static analysis"
        VERBATIM
    )
else()
    message(STATUS "clang-tidy not found; 'tidy' target will fail until installed.")

    add_custom_target(tidy
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found in PATH"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

if(IO_RIPPER_CORE_BUILD_SHARED)
    set(IO_RIPPER_CORE_LIBRARY_TYPE SHARED)
else()
    set(IO_RIPPER_CORE_LIBRARY_TYPE STATIC)
endif()

add_library(io_ripper_core ${IO_RIPPER_CORE_LIBRARY_TYPE}
    ${IO_RIPPER_CORE_SOURCES}
)

add_library(io_ripper_core::io_ripper_core ALIAS io_ripper_core)

target_compile_features(io_ripper_core PUBLIC cxx_std_23)

target_include_directories(io_ripper_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(io_ripper_core PROPERTIES
    OUTPUT_NAME io_ripper_core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS io_ripper_core
    EXPORT io_ripper_coreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT io_ripper_coreTargets
    FILE io_ripper_coreTargets.cmake
    NAMESPACE io_ripper_core::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/io_ripper_core
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/io_ripper_coreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/io_ripper_coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/io_ripper_coreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/io_ripper_core
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/io_ripper_coreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/io_ripper_coreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/io_ripper_core
)

if(IO_RIPPER_CORE_ENABLE_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )

    FetchContent_MakeAvailable(Catch2)

    set(BUILD_TESTING ON CACHE BOOL "Enable testing" FORCE)
    add_subdirectory(tests)
endif()
