include(FetchContent)

# Fetch GoogleTest centrally
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

# Fetch Google Benchmark centrally
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz
)
FetchContent_MakeAvailable(googlebenchmark)
