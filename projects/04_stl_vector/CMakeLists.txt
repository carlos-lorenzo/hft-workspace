# Define the header-only target
add_library(order_book_l2 INTERFACE)
target_include_directories(order_book_l2 INTERFACE include/)

# Test Executable
add_executable(test_order_book_l2 tests/order_book_l2_test.cpp)
target_link_libraries(test_order_book_l2 PRIVATE order_book_l2 GTest::gtest_main)

# Benchmark Executable
add_executable(bench_order_book_l2 benchmarks/order_book_l2_bench.cpp)
target_link_libraries(bench_order_book_l2 PRIVATE order_book_l2 benchmark::benchmark)