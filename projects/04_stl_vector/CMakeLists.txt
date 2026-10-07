# Define the header-only target
add_library(stl_vector INTERFACE)
target_include_directories(stl_vector INTERFACE include/)

# Test Executable
add_executable(test_stl_vector tests/stl_vector_test.cpp)
# Changed "GTest::gtest_main" to "gtest_main"
target_link_libraries(test_stl_vector PRIVATE stl_vector gtest_main)

# Benchmark Executable
add_executable(bench_stl_vector benchmarks/stl_vector_bench.cpp)
# Changed "benchmark::benchmark" to "benchmark"
target_link_libraries(bench_stl_vector PRIVATE stl_vector benchmark)