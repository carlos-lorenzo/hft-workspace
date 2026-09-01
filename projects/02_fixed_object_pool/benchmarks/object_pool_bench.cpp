#include <benchmark/benchmark.h>
#include <array>
#include <fixed_object_pool.hpp>

struct MockOrder { uint64_t id; double price; uint32_t qty; };
constexpr size_t capacity = 1024;

static void BM_RingBuffer_PushPop(benchmark::State& state) {
    // Setup state before hot loop
    FixedObjectPool<MockOrder, capacity> pool;


    // Hot execution loop measured by time/cycles
    for (auto _ : state) {
        auto ptr = pool.allocate(1, 2, 3);
        benchmark::DoNotOptimize(ptr);
        pool.deallocate(ptr);
    }
}
BENCHMARK(BM_RingBuffer_PushPop);

BENCHMARK_MAIN();