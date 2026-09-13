#include <benchmark/benchmark.h>
#include <array>
#include <stl_vector.hpp>

constexpr size_t capacity = 1024;

static void BM_RingBuffer_PushPop(benchmark::State& state) {
    // Setup state before hot loop



    // Hot execution loop measured by time/cycles
    for (auto _ : state) {

    }
}
BENCHMARK(BM_RingBuffer_PushPop);

BENCHMARK_MAIN();