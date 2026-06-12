add_executable(io_ripper_core_tests
    file_reader_tests.cpp
    file_writer_tests.cpp
    memory_reader_tests.cpp
    memory_writer_tests.cpp
    numeric_cast_tests.cpp
)

target_compile_features(io_ripper_core_tests PRIVATE cxx_std_23)

target_include_directories(io_ripper_core_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(io_ripper_core_tests
    PRIVATE
        io_ripper_core
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(io_ripper_core_tests)
