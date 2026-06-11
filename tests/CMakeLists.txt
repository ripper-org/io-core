add_executable(io_ripper_core_tests
    file_reader_tests.cpp
    file_writer_tests.cpp
)

target_compile_features(io_ripper_core_tests PRIVATE cxx_std_23)

target_link_libraries(io_ripper_core_tests
    PRIVATE
        io_ripper_core
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(io_ripper_core_tests)
