# Define the header-only target
add_library(fixed_ring_buffer INTERFACE)
target_include_directories(fixed_ring_buffer INTERFACE include/)

# Test Executable
add_executable(test_fixed_ring_buffer tests/ring_buffer_test.cpp)
target_link_libraries(test_fixed_ring_buffer PRIVATE fixed_ring_buffer GTest::gtest_main)

# Benchmark Executable
add_executable(bench_fixed_ring_buffer benchmarks/ring_buffer_bench.cpp)
target_link_libraries(bench_fixed_ring_buffer PRIVATE fixed_ring_buffer benchmark::benchmark)