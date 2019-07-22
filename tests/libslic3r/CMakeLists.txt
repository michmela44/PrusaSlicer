find_package(Catch2 REQUIRED)

add_executable(libslic3r_tests 
    test_harness.cpp
    fill.cpp
)

target_include_directories(libslic3r_tests 
    PUBLIC ${PROJECT_SOURCE_DIR}/src  ${PROJECT_SOURCE_DIR}/src/libslic3r
)

target_link_libraries(libslic3r_tests Catch2::Catch2 libslic3r)

include(Catch)
catch_discover_tests(libslic3r_tests TEST_PREFIX libslic3r-core.)
