cmake_minimum_required(VERSION 3.23)
project(htf_workspace LANGUAGES CXX)

# Global Compiler Flags & Standard Enforcement
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# HFT Performance & Diagnostic Flags
add_compile_options(
        -march=native
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

# Fetch Shared External Dependencies (GTest, Google Benchmark)
include(cmake/Dependencies.cmake)



# Register Subprojects
add_subdirectory(projects/01_fixed_ring_buffer)
add_subdirectory(projects/02_fixed_object_pool)
add_subdirectory(projects/03_order_book_l2)
add_subdirectory(projects/04_stl_vector)
add_subdirectory(projects/05_naive_orderbook_l3)