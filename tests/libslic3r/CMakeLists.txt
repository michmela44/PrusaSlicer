find_package(Catch2 REQUIRED)

add_executable(libslic3r_tests 
    test_harness.cpp
    test_fill.cpp
    test_gcodewriter.cpp
    test_options.hpp
)

set_target_properties(libslic3r_tests PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(libslic3r_tests 
    PUBLIC ${PROJECT_SOURCE_DIR}/src  ${PROJECT_SOURCE_DIR}/src/libslic3r
)

target_link_libraries(libslic3r_tests Catch2::Catch2 libslic3r)

include(Catch)
catch_discover_tests(libslic3r_tests TEST_PREFIX libslic3r-core.)
