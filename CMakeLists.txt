cmake_minimum_required(VERSION 3.20)

project(io_ripper_core
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE IO_RIPPER_CORE_SOURCES
    CONFIGURE_DEPENDS
    src/core/*.cpp
)

add_library(io_ripper_core SHARED
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
