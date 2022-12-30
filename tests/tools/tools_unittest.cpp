#define BOOST_TEST_MODULE slf_tools_unittest
#include "tools/fenwick.h"
#include "tools/shared_object_memory_pool.h"
#include "tools/timer.h"
#include "common.h"
#include <boost/test/included/unit_test.hpp>
#include <random>
#include <thread>
#include <vector>

namespace slf::test
{

BOOST_AUTO_TEST_CASE(scope_timer_test)
{
    double ms{0};
    {
        scope_timer st(ms);
        sleep_for_us(3000);
    }
    BOOST_CHECK(ms > 3);
}

void fenwick_test_func(int len, int times)
{
    // in slf, offset is always one.
    fenwick<int> fw(len, 1);
    std::vector<int> presum(len, 0);
    for (int i = 0; i < times; ++i)
    {
        int pos = abs(rand()) % len;
        int val = abs(rand());
        fw.add(pos, val);
        for (int i = pos; i < len; ++i)
            presum[i] += val;
        for (int i = 0; i < len; ++i)
        {
            BOOST_REQUIRE_EQUAL(fw.query(i), presum[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(fenwick_test)
{
    srand(time(0));
    fenwick_test_func(2, 5);
    fenwick_test_func(10, 20);
    fenwick_test_func(128, 300);
    fenwick_test_func(567, 1254);
}

BOOST_AUTO_TEST_CASE(shared_object_memory_pool_test1)
{
    constexpr size_t pool_size = 10;
    shared_object_memory_pool<int> pool(pool_size);
    std::vector<int*> ptrs;
    for (size_t i = 0; i < pool_size; i++)
    {
        BOOST_REQUIRE_EQUAL(true, pool.all_objects_are_usable());
        auto ptr = pool.get();
        BOOST_REQUIRE_EQUAL(2, ptr.use_count());
        *ptr = i + 1;
        ptrs.push_back(ptr.get());
    }
    std::vector<std::shared_ptr<int>> sptrs;
    for (size_t i = 0; i < pool_size; i++)
    {
        auto ptr = pool.get();
        BOOST_REQUIRE_EQUAL(2, ptr.use_count());
        BOOST_REQUIRE_EQUAL(ptr.get(), ptrs[i]);
        sptrs.emplace_back(ptr);
    }
    BOOST_REQUIRE_EQUAL(false, pool.all_objects_are_usable());
    auto ptr = pool.get();
    BOOST_REQUIRE_EQUAL(true, std::all_of(sptrs.begin(), sptrs.end(),
                                          [&](std::shared_ptr<int>& p)
                                          { return p.get() != ptr.get(); }));
}

BOOST_AUTO_TEST_CASE(shared_object_memory_pool_test2)
{
    constexpr size_t pool_size = 100;
    constexpr size_t thread_number = 8;
    constexpr size_t op_times = 10000;
    shared_object_memory_pool<std::atomic_int> pool(pool_size);
    std::atomic_bool run{false};
    std::vector<std::thread> threads(thread_number);
    auto met_error = std::make_unique<std::atomic_bool[]>(thread_number);
    for (size_t i = 0; i < thread_number; i++)
    {
        met_error[i] = false;
        threads[i] = std::thread(
            [&](size_t id)
            {
                while (run.load() == false)
                    ;
                for (size_t i = 0; i < op_times; i++)
                {
                    auto ptr = pool.get();
                    if (ptr.use_count() != 2)
                        met_error[id] = true;
                    sleep_for_us(1);
                    *ptr += 1;
                }
            },
            i);
    }
    run = true;
    for (size_t i = 0; i < thread_number; i++)
    {
        if (threads[i].joinable())
            threads[i].join();
        BOOST_CHECK_EQUAL(false, met_error[i].load());
    }
    BOOST_REQUIRE_EQUAL(true, pool.all_objects_are_usable());
    size_t res = 0;
    for (size_t i = 0; i < pool_size; i++)
    {
        auto ptr = pool.get();
        res += *ptr;
    }
    BOOST_CHECK_EQUAL(res, thread_number * op_times);
}

} // namespace slf::test
