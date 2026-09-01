# Define the header-only target
add_library(fixed_object_pool INTERFACE)
target_include_directories(fixed_object_pool INTERFACE include/)

# Test Executable
add_executable(test_fixed_object_pool tests/object_pool_test.cpp)
target_link_libraries(test_fixed_object_pool PRIVATE fixed_object_pool GTest::gtest_main)

# Benchmark Executable
add_executable(bench_fixed_object_pool benchmarks/object_pool_bench.cpp)
target_link_libraries(bench_fixed_object_pool PRIVATE fixed_object_pool benchmark::benchmark)