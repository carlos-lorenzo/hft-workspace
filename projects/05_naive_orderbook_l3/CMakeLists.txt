# Define the header-only target
add_library(naive_orderbook_l3 INTERFACE)
target_include_directories(naive_orderbook_l3 INTERFACE include/)

# Test Executable
add_executable(test_naive_orderbook_l3 tests/naive_orderbook_l3_test.cpp)
target_link_libraries(test_naive_orderbook_l3 PRIVATE naive_orderbook_l3 GTest::gtest_main)

# Benchmark Executable
add_executable(bench_naive_orderbook_l3 benchmarks/naive_orderbook_l3_bench.cpp)
target_link_libraries(bench_naive_orderbook_l3 PRIVATE naive_orderbook_l3 benchmark::benchmark)