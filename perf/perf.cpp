#include "predefined.h"
#include "si/tasks.h"
#include <benchmark/benchmark.h>
#include <mutex>
#include <numeric>

template <typename T>
void tasks_test_f(benchmark::State& state)
{
    auto len = state.range(0);
    std::unique_ptr<slf::node_id_t[]> ptr_(new slf::node_id_t[len]);
    std::iota(ptr_.get(), ptr_.get() + len, 0);
    for (auto _ : state)
    {
        T tasks_;
        tasks_.set_tasks(ptr_.get(), ptr_.get() + len);
        slf::node_id_t id;
        benchmark::ClobberMemory();
        while (tasks_.get_task(id))
        {
            auto s = tasks_.size();
            auto e = tasks_.empty();
            benchmark::DoNotOptimize(id);
            benchmark::DoNotOptimize(s);
            benchmark::DoNotOptimize(e);
        }
    }
}

static void BM_tasks_seq_test(benchmark::State& state){
    tasks_test_f<slf::tasks<false>>(state);
}
BENCHMARK(BM_tasks_seq_test) 
    ->Range(4, 2 << 13);

static void BM_tasks_lockfree_test(benchmark::State& state){
    tasks_test_f<slf::tasks<true,true>>(state);
}
BENCHMARK(BM_tasks_lockfree_test)
    ->Range(4, 2 << 13);

static void BM_tasks_mutex_test(benchmark::State& state){
    tasks_test_f<slf::tasks<true,false>>(state);
}
BENCHMARK(BM_tasks_mutex_test)
    ->Range(4, 2 << 13);

BENCHMARK_MAIN();
