#include <benchmark/benchmark.h>
#include <array>
#include <order_book_l2.hpp>

struct MockOrder { uint64_t id; double price; uint32_t qty; };
constexpr size_t capacity = 1024;

static void BM_RingBuffer_PushPop(benchmark::State& state) {
    // Setup state before hot loop



    // Hot execution loop measured by time/cycles
    for (auto _ : state) {

    }
}
BENCHMARK(BM_RingBuffer_PushPop);

BENCHMARK_MAIN();