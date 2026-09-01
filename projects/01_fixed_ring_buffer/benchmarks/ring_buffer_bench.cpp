#include <benchmark/benchmark.h>
#include <array>
#include <fixed_ring_buffer.hpp>

struct MockOrder { uint64_t id; double price; uint32_t qty; };
constexpr size_t capacity = 1024;

static void BM_RingBuffer_PushPop(benchmark::State& state) {
    // Setup state before hot loop
    FixedRingBuffer<MockOrder, capacity> buffer;
    MockOrder item = {42, 100.0, 10};
    MockOrder out_val = {0, 0.0, 0};

    // Hot execution loop measured by time/cycles
    for (auto _ : state) {
        benchmark::DoNotOptimize(buffer.try_push(std::move(item)));
        benchmark::DoNotOptimize(buffer.try_pop(out_val));
    }
}
BENCHMARK(BM_RingBuffer_PushPop);

BENCHMARK_MAIN();