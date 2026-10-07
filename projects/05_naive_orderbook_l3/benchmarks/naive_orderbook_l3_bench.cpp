
#include <benchmark/benchmark.h>
#include <naive_orderbook_l3.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using Nanoseconds = std::chrono::nanoseconds;

struct Order {
    uint32_t uuid;
    uint32_t qty;
    uint32_t original_qty;
    bool side;
};

using Book = NaiveOrderbookL3Side<
    Order,
    uint32_t,
    uint32_t,
    uint32_t,
    true
>;

constexpr uint32_t BASE_PRICE = 10000;
constexpr uint32_t BASE_QTY = 100;

// ---------------------------------------------------------
// Statistics
// ---------------------------------------------------------

struct LatencyStats {
    double mean = 0;
    double stddev = 0;
    double p50 = 0;
    double p95 = 0;
    double p99 = 0;
    double p999 = 0;
    double max = 0;
};

double percentile(const std::vector<double>& sorted, double p) {
    if (sorted.empty()) {
        return 0;
    }

    const size_t index = static_cast<size_t>(
        std::ceil(p * sorted.size())
    ) - 1;

    return sorted[std::min(index, sorted.size() - 1)];
}

LatencyStats calculate_stats(std::vector<double>& samples) {
    LatencyStats stats{};

    if (samples.empty()) {
        return stats;
    }

    std::sort(samples.begin(), samples.end());

    const double sum = std::accumulate(
        samples.begin(),
        samples.end(),
        0.0
    );

    stats.mean = sum / samples.size();
    stats.p50 = percentile(samples, 0.50);
    stats.p95 = percentile(samples, 0.95);
    stats.p99 = percentile(samples, 0.99);
    stats.p999 = percentile(samples, 0.999);
    stats.max = samples.back();

    double squared_sum = 0;

    for (double x : samples) {
        const double diff = x - stats.mean;
        squared_sum += diff * diff;
    }

    stats.stddev = std::sqrt(
        squared_sum / samples.size()
    );

    return stats;
}

void report_stats(
    benchmark::State& state,
    const std::vector<double>& samples
) {
    if (samples.empty()) {
        return;
    }

    auto copy = samples;
    const auto stats = calculate_stats(copy);

    state.counters["mean_ns"] = stats.mean;
    state.counters["stddev_ns"] = stats.stddev;
    state.counters["p50_ns"] = stats.p50;
    state.counters["p95_ns"] = stats.p95;
    state.counters["p99_ns"] = stats.p99;
    state.counters["p999_ns"] = stats.p999;
    state.counters["max_ns"] = stats.max;

    state.counters["samples"] =
        static_cast<double>(samples.size());
}

template <typename Fn>
double measure_ns(Fn&& fn) {
    const auto start = Clock::now();

    benchmark::DoNotOptimize(fn());

    const auto end = Clock::now();

    return std::chrono::duration<double, std::nano>(
        end - start
    ).count();
}

// ---------------------------------------------------------
// Throughput benchmarks
// ---------------------------------------------------------

// Add N orders to a fresh book per iteration.
// N is controlled through BENCHMARK(...)->Arg(N).

    static void BM_AddThroughput(benchmark::State& state) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE + i % 100);
        }

        benchmark::DoNotOptimize(book.order_count());
        benchmark::ClobberMemory();

        // No PauseTiming() here.
    }

    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_AddThroughput)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Cancel throughput
// ---------------------------------------------------------

static void BM_CancelThroughput(benchmark::State& state) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            book.cancel(i + 1);
        }

        benchmark::DoNotOptimize(book.empty());
    }

    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_CancelThroughput)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Partial execution throughput
// ---------------------------------------------------------

static void BM_PartialExecuteThroughput(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            book.execute(i + 1, 1);
        }

        benchmark::DoNotOptimize(book.order_count());
    }

    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_PartialExecuteThroughput)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Full execution throughput
// ---------------------------------------------------------

static void BM_FullExecuteThroughput(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            book.execute(i + 1, BASE_QTY);
        }

        benchmark::DoNotOptimize(book.empty());
    }

    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_FullExecuteThroughput)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Best price throughput
// ---------------------------------------------------------

static void BM_BestPriceThroughput(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    Book book;

    for (uint32_t i = 0; i < N; ++i) {
        book.add(i + 1, BASE_QTY, BASE_PRICE + i);
    }

    for (auto _ : state) {
        auto price = book.best_price();

        benchmark::DoNotOptimize(price);
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_BestPriceThroughput)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Quantity-at-price throughput
// ---------------------------------------------------------

static void BM_QuantityAtPriceThroughput(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    Book book;

    for (uint32_t i = 0; i < N; ++i) {
        book.add(i + 1, BASE_QTY, BASE_PRICE);
    }

    for (auto _ : state) {
        auto qty = book.quantity_at(BASE_PRICE);

        benchmark::DoNotOptimize(qty);
    }

    state.SetItemsProcessed(state.iterations());
}

// One price level containing many orders.
BENCHMARK(BM_QuantityAtPriceThroughput)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Latency benchmarks
// ---------------------------------------------------------

// These benchmarks intentionally measure individual operations.
// Clock and instrumentation overhead are included.

static void BM_AddLatency(benchmark::State& state) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    std::vector<double> samples;
    samples.reserve(100000);

    uint32_t next_id = 1;

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            const uint32_t id = next_id++;

            samples.push_back(measure_ns([&]() {
                book.add(id, BASE_QTY, BASE_PRICE);
                return book.order_count();
            }));
        }

        benchmark::DoNotOptimize(book.order_count());
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_AddLatency)
    ->Arg(1000)
    ->Iterations(100);

// ---------------------------------------------------------

static void BM_CancelLatency(benchmark::State& state) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    std::vector<double> samples;
    samples.reserve(100000);

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            samples.push_back(measure_ns([&]() {
                book.cancel(i + 1);
                return book.order_count();
            }));
        }
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_CancelLatency)
    ->Arg(1000)
    ->Iterations(100);

// ---------------------------------------------------------

static void BM_PartialExecuteLatency(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    std::vector<double> samples;
    samples.reserve(100000);

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            samples.push_back(measure_ns([&]() {
                book.execute(i + 1, 1);
                return book.order_count();
            }));
        }
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_PartialExecuteLatency)
    ->Arg(1000)
    ->Iterations(100);

// ---------------------------------------------------------

static void BM_FullExecuteLatency(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    std::vector<double> samples;
    samples.reserve(100000);

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 0; i < N; ++i) {
            book.add(i + 1, BASE_QTY, BASE_PRICE);
        }

        state.ResumeTiming();

        for (uint32_t i = 0; i < N; ++i) {
            samples.push_back(measure_ns([&]() {
                book.execute(i + 1, BASE_QTY);
                return book.order_count();
            }));
        }
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_FullExecuteLatency)
    ->Arg(1000)
    ->Iterations(100);

// ---------------------------------------------------------

static void BM_BestPriceLatency(
    benchmark::State& state
) {
    const uint32_t N = static_cast<uint32_t>(state.range(0));

    Book book;

    for (uint32_t i = 0; i < N; ++i) {
        book.add(i + 1, BASE_QTY, BASE_PRICE + i);
    }

    std::vector<double> samples;
    samples.reserve(100000);

    for (auto _ : state) {
        samples.push_back(measure_ns([&]() {
            return book.best_price();
        }));
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_BestPriceLatency)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ---------------------------------------------------------
// Mixed workload
// ---------------------------------------------------------

static void BM_MixedWorkload(benchmark::State& state) {
    constexpr uint32_t INITIAL_ORDERS = 1000;

    std::vector<double> samples;
    samples.reserve(100000);

    uint32_t next_id = INITIAL_ORDERS + 1;

    for (auto _ : state) {
        state.PauseTiming();

        Book book;

        for (uint32_t i = 1; i <= INITIAL_ORDERS; ++i) {
            book.add(i, BASE_QTY, BASE_PRICE + i % 20);
        }

        uint32_t active_id = 1;

        state.ResumeTiming();

        for (uint32_t i = 0; i < 1000; ++i) {
            const uint32_t operation = i % 4;

            if (operation == 0) {
                const uint32_t id = next_id++;

                samples.push_back(measure_ns([&]() {
                    book.add(id, BASE_QTY, BASE_PRICE + id % 20);
                    return book.order_count();
                }));
            }
            else if (operation == 1) {
                samples.push_back(measure_ns([&]() {
                    book.execute(active_id, 1);
                    return book.order_count();
                }));
            }
            else if (operation == 2) {
                samples.push_back(measure_ns([&]() {
                    book.cancel(active_id);
                    return book.order_count();
                }));
            }
            else {
                samples.push_back(measure_ns([&]() {
                    return book.best_price();
                }));
            }

            active_id = active_id % INITIAL_ORDERS + 1;
        }

        benchmark::DoNotOptimize(book.order_count());
    }

    state.SetItemsProcessed(samples.size());
    report_stats(state, samples);
}

BENCHMARK(BM_MixedWorkload)
    ->Iterations(100);

} // namespace

BENCHMARK_MAIN();