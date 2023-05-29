#define BOOST_TEST_MODULE si_unittest
#include "config/slf_config.h"
#include "si/parallel_subgraph_isomorphism.h"
#include "si/results_taker.h"
#include "si/sequential_subgraph_isomorphism.h"
#include "si/state.h"
#include "si/tasks.h"
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/test/included/unit_test.hpp>
#include <future>
#include <thread>
namespace slf::test
{
class si_ut_prepare
{
public:
    si_ut_prepare()
    {
        boost::log::core::get()->set_filter(
            boost::log::trivial::severity >=
            boost::log::trivial::severity_level::info);
    }
};
BOOST_GLOBAL_FIXTURE(si_ut_prepare);
BOOST_AUTO_TEST_CASE(results_taker_sequential)
{
    results_taker<false> rt(5);
    BOOST_CHECK_EQUAL(rt.max_log_results(), 5);
    std::vector<node_id_t> mapping{1, 2, 3, 4};
    for (size_t i = 0; i < 1000; i++)
    {
        BOOST_CHECK_EQUAL(rt(mapping), i + 1);
        BOOST_CHECK_EQUAL(rt.result_number(), i + 1);
    }
}

BOOST_AUTO_TEST_CASE(results_taker_multithread)
{
    results_taker<true> rt(1);
    BOOST_CHECK_EQUAL(rt.max_log_results(), 1);
    std::vector<node_id_t> mapping{1, 2, 3, 4};
    constexpr int threads_number = 8;
    constexpr int results_number_per_thread = 5000;
    std::vector<std::thread> threads(threads_number);
    std::vector<std::vector<size_t>> cnts(threads_number);
    for (size_t i = 0; i < threads_number; i++)
    {
        threads[i] = std::thread(
            [&, id = i]()
            {
                cnts[id].reserve(results_number_per_thread);
                for (size_t i = 0; i < results_number_per_thread; i++)
                {
                    cnts[id].emplace_back(rt(mapping));
                }
            });
    }
    std::vector<size_t> res;
    for (size_t i = 0; i < threads_number; i++)
    {
        if (threads[i].joinable())
            threads[i].join();
        res.insert(res.end(), cnts[i].begin(), cnts[i].end());
    }
    sort(res.begin(), res.end());
    for (size_t i = 0; i < threads_number * results_number_per_thread; i++)
    {
        BOOST_REQUIRE_EQUAL(res[i], i + 1);
    }
}

template <bool multi_thread>
auto prepare_tasks(size_t len, bool inc, std::unique_ptr<node_id_t[]>& ids,
                   size_t bgn = 0)
{
    static std::vector<source_edge_t> source_edges_;
    static std::vector<target_edge_t> target_edges_;
    source_edges_.clear();
    target_edges_.clear();
    auto tasks_ptr_ = std::make_unique<tasks<multi_thread>>();
    auto& tasks_ = *tasks_ptr_;
    ids = std::make_unique<node_id_t[]>(len);
    auto raw_ptr = ids.get();
    std::iota(raw_ptr, raw_ptr + len, bgn);

    if (!inc)
        std::reverse(raw_ptr, raw_ptr + len);
    tasks_.set_tasks(raw_ptr, raw_ptr + len);
    return tasks_ptr_;
}

void sequential_task_test(size_t len, bool inc)
{
    std::unique_ptr<node_id_t[]> ids;
    auto tasks_ptr_ = prepare_tasks<false>(len, inc, ids);
    auto& tasks_ = *tasks_ptr_;
    node_id_t id;
    for (size_t i = 0; i < len; i++)
    {
        node_id_t exp_id = (inc) ? i : (len - 1 - i);
        BOOST_REQUIRE_EQUAL(tasks_.size(), len - i);
        BOOST_REQUIRE(tasks_.get_task(id));
        BOOST_REQUIRE_EQUAL(exp_id, id);
    }
    BOOST_REQUIRE(!tasks_.get_task(id));
    BOOST_REQUIRE(tasks_.empty());
}

void multithread_task_test(size_t len, bool inc, size_t threads_number)
{
    std::unique_ptr<node_id_t[]> ids;
    auto tasks_ptr_ = prepare_tasks<true>(len, inc, ids);
    auto& tasks_ = *tasks_ptr_;
    std::vector<std::thread> threads(threads_number);
    std::vector<std::vector<size_t>> res(threads_number);
    for (size_t i = 0; i < threads_number; i++)
    {
        threads[i] = std::thread(
            [&, id = i]()
            {
                res[id].reserve(len);
                node_id_t task;
                while (true)
                {
                    if (!tasks_.get_task(task))
                        break;
                    res[id].emplace_back(task);
                }
            });
    }
    std::vector<size_t> r;
    for (size_t i = 0; i < threads_number; i++)
    {
        if (threads[i].joinable())
            threads[i].join();
        if (inc)
            BOOST_REQUIRE(std::is_sorted(res[i].begin(), res[i].end()));
        else
            BOOST_REQUIRE(std::is_sorted(res[i].begin(), res[i].end(),
                                         greater<node_id_t>()));

        r.insert(r.end(), res[i].begin(), res[i].end());
    }
    if (r.size() != len)
    {
        std::map<node_id_t, int> mp;
        for (const auto& var : r)
        {
            mp[var]++;
            if (mp[var] > 1)
            {
                cout << "double exist" << var << endl;
            }
        }
    }
    BOOST_REQUIRE_EQUAL(r.size(), len);
    sort(r.begin(), r.end());
    for (size_t i = 0; i < len; i++)
    {
        BOOST_REQUIRE_EQUAL(r[i], i);
    }
}

void shared_tasks_test(size_t len, bool inc, size_t pre_use)
{
    assert(pre_use < len);
    std::unique_ptr<node_id_t[]> ids1, ids2;
    auto tasks1_ptr_ = prepare_tasks<false>(len, inc, ids1);
    auto tasks2_ptr_ = prepare_tasks<false>(len, inc, ids2);
    node_id_t t1, t2;
    for (size_t i = 0; i < pre_use; i++)
    {
        BOOST_REQUIRE(tasks1_ptr_->get_task(t1));
        BOOST_REQUIRE(tasks2_ptr_->get_task(t2));
        BOOST_REQUIRE_EQUAL(t1, t2);
    }
    shared_tasks st;
    st.transfer_tasks(*tasks2_ptr_, {1, 2, 3});
    BOOST_REQUIRE(st.target_match_sequence() ==
                  std::vector<node_id_t>({1, 2, 3}));
    while (!tasks1_ptr_->empty())
    {
        BOOST_REQUIRE(tasks1_ptr_->get_task(t1));
        BOOST_REQUIRE(st.get_task(t2));
        BOOST_REQUIRE_EQUAL(t1, t2);
    }
    BOOST_REQUIRE(st.empty());
}

BOOST_AUTO_TEST_CASE(sequential_task)
{
    size_t lens[4] = {2, 8, 123, 890};
    for (auto l : lens)
    {
        sequential_task_test(l, true);
        sequential_task_test(l, false);
    }
}

BOOST_AUTO_TEST_CASE(multi_thread_task)
{
    size_t lens[4] = {8, 123, 890, 3456};
    size_t thread_number[3] = {1, 4, 8};
    for (auto l : lens)
    {
        for (auto t : thread_number)
        {
            multithread_task_test(l, true, t);
            multithread_task_test(l, false, t);
        }
    }
}

BOOST_AUTO_TEST_CASE(shared_tasks_unittest)
{
    size_t lens[4] = {8, 123, 890, 3456};
    float pre_use_perc[5] = {0, 0.2, 0.5, 0.7, 0.9};
    for (auto len : lens)
    {
        for (auto p : pre_use_perc)
        {
            shared_tasks_test(len, true, len * p);
            shared_tasks_test(len, false, len * p);
        }
    }
}

/*
0 <-> 1
0  -> 2
2 <-> 2
label: node 2 is 1 and other are 0.
*/
std::unique_ptr<graph_t> create_query_graph()
{
    auto ret = std::make_unique<graph_t>();
    ret->set_node_number(3);
    ret->set_node_label(0, 0);
    ret->set_node_label(1, 0);
    ret->set_node_label(2, 1);
    ret->add_edge(0, 1);
    ret->add_edge(1, 0);
    ret->add_edge(0, 2);
    ret->add_edge(2, 0);
    ret->add_edge(2, 2);
    ret->sort_edges();
    return ret;
}

std::unique_ptr<graph_t> create_target_graph()
{
    auto ret = std::make_unique<graph_t>();
    std::vector<int> group_node = {3, 3, 3, 4};
    int group = 0;
    ret->set_node_number(
        std::accumulate(group_node.begin(), group_node.end(), 0));
    int id_offset = 0;
    // group 1: match
    ret->set_node_label(0 + id_offset, 0);
    ret->set_node_label(1 + id_offset, 0);
    ret->set_node_label(2 + id_offset, 1);
    ret->add_edge(0 + id_offset, 1 + id_offset);
    ret->add_edge(1 + id_offset, 0 + id_offset);
    ret->add_edge(0 + id_offset, 2 + id_offset);
    ret->add_edge(2 + id_offset, 0 + id_offset);
    ret->add_edge(2 + id_offset, 2 + id_offset);
    id_offset += group_node[group++];

    // group 2: label unmatch
    ret->set_node_label(0 + id_offset, 1);
    ret->set_node_label(1 + id_offset, 0);
    ret->set_node_label(2 + id_offset, 1);
    ret->add_edge(0 + id_offset, 1 + id_offset);
    ret->add_edge(1 + id_offset, 0 + id_offset);
    ret->add_edge(0 + id_offset, 2 + id_offset);
    ret->add_edge(2 + id_offset, 0 + id_offset);
    ret->add_edge(2 + id_offset, 2 + id_offset);
    id_offset += group_node[group++];

    // group 3: sele_loop missing
    ret->set_node_label(0 + id_offset, 0);
    ret->set_node_label(1 + id_offset, 0);
    ret->set_node_label(2 + id_offset, 1);
    ret->add_edge(0 + id_offset, 1 + id_offset);
    ret->add_edge(1 + id_offset, 0 + id_offset);
    ret->add_edge(0 + id_offset, 2 + id_offset);
    ret->add_edge(2 + id_offset, 0 + id_offset);
    id_offset += group_node[group++];

    // group 4: edge missing
    ret->set_node_label(0 + id_offset, 0);
    ret->set_node_label(1 + id_offset, 0);
    ret->set_node_label(2 + id_offset, 1);
    ret->set_node_label(3 + id_offset, 0);
    ret->add_edge(0 + id_offset, 1 + id_offset);
    ret->add_edge(0 + id_offset, 2 + id_offset);
    ret->add_edge(2 + id_offset, 0 + id_offset);
    ret->add_edge(2 + id_offset, 2 + id_offset);
    ret->add_edge(3 + id_offset, 0 + id_offset);
    ret->sort_edges();
    return ret;
}

BOOST_AUTO_TEST_CASE(compress_sub_state_test1)
{
    auto graph = create_query_graph();
    compress_sub_state sub_state_(graph->node_number());
    std::vector<node_id_t> ms = {0, 1, 2};
    int depth = 0;
    // depth 0
    for (size_t i = 0; i < 3; i++)
    {
        BOOST_REQUIRE(!sub_state_.in_depth(i, depth));
        BOOST_REQUIRE(!sub_state_.out_depth(i, depth));
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_in(i, depth), false);
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_out(i, depth), false);
        BOOST_REQUIRE_EQUAL(sub_state_.unmapped(i, depth), true);
    }

    sub_state_.add_node(graph->get_node(ms[depth]));
    ++depth;
    // depth 1
    for (size_t i = 0; i < 3; i++)
    {
        bool bool_exp = i;
        auto in_dep = sub_state_.in_depth(i, depth);
        auto out_dep = sub_state_.out_depth(i, depth);
        BOOST_REQUIRE_EQUAL(in_dep.has_value(), bool_exp);
        BOOST_REQUIRE_EQUAL(out_dep.has_value(), bool_exp);
        if (bool_exp)
        {
            BOOST_REQUIRE_EQUAL(in_dep.value(), 1);
            BOOST_REQUIRE_EQUAL(out_dep.value(), 1);
        }
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_in(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_out(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.unmapped(i, depth), bool_exp);
    }

    sub_state_.add_node(graph->get_node(ms[depth]));
    ++depth;
    // depth 2
    for (size_t i = 0; i < 3; i++)
    {
        bool bool_exp = (i == 2);
        auto in_dep = sub_state_.in_depth(i, depth);
        auto out_dep = sub_state_.out_depth(i, depth);
        BOOST_REQUIRE_EQUAL(in_dep.has_value(), i > 0);
        BOOST_REQUIRE_EQUAL(out_dep.has_value(), i > 0);
        if (i > 0)
        {
            BOOST_REQUIRE_EQUAL(in_dep.value(), 1);
            BOOST_REQUIRE_EQUAL(out_dep.value(), 1);
        }
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_in(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_out(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.unmapped(i, depth), bool_exp);
    }

    sub_state_.add_node(graph->get_node(ms[depth]));
    ++depth;
    // depth 3
    for (size_t i = 0; i < 3; i++)
    {
        bool bool_exp = false;
        auto in_dep = sub_state_.in_depth(i, depth);
        auto out_dep = sub_state_.out_depth(i, depth);
        BOOST_REQUIRE_EQUAL(in_dep.has_value(), i > 0);
        BOOST_REQUIRE_EQUAL(out_dep.has_value(), i > 0);
        if (i > 0)
        {
            BOOST_REQUIRE_EQUAL(in_dep.value(), 1);
            BOOST_REQUIRE_EQUAL(out_dep.value(), 1);
        }
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_in(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.in_set_out(i, depth), bool_exp);
        BOOST_REQUIRE_EQUAL(sub_state_.unmapped(i, depth), bool_exp);
    }
}

BOOST_AUTO_TEST_CASE(state_test1)
{
    auto query = create_query_graph();
    auto target = create_target_graph();
    std::vector<node_id_t> ms = {0, 1, 2};
    auto query_state_ptr = std::shared_ptr<const query_complete_sub_state>(
        new query_complete_sub_state(*query, ms));
    auto preset_tasks_ptr = preset_tasks::create_preset_tasks(*target);
    state state_(*query, *target, query_state_ptr, preset_tasks_ptr);
    auto check = [&](int offset)
    {
        BOOST_REQUIRE(state_.add_pair_if_abbable(0, 0 + offset));
        BOOST_REQUIRE(state_.add_pair_if_abbable(1, 1 + offset));
        BOOST_REQUIRE(state_.add_pair_if_abbable(2, 2 + offset));
        BOOST_REQUIRE(state_.cover_query_graph());
        state_.remove_pair();
        state_.remove_pair();
        state_.remove_pair();
    };

    int offset = 0;
    // group 1
    check(offset);

    // group 2
    offset += 3;
    BOOST_REQUIRE(!state_.add_pair_if_abbable(0, 0 + offset));
    target->set_node_label(0 + offset, 0);
    check(offset);

    // gropu 3
    offset += 3;
    BOOST_REQUIRE(state_.add_pair_if_abbable(0, 0 + offset));
    BOOST_REQUIRE(state_.add_pair_if_abbable(1, 1 + offset));
    BOOST_REQUIRE(!state_.add_pair_if_abbable(2, 2 + offset));
    state_.remove_pair();
    state_.remove_pair();
    target->add_edge(2 + offset, 2 + offset);
    target->sort_edges();
    check(offset);

    // gropu 4
    offset += 3;
    BOOST_REQUIRE(state_.add_pair_if_abbable(0, 0 + offset));
    BOOST_REQUIRE(!state_.add_pair_if_abbable(1, 1 + offset));
    state_.remove_pair();
    target->add_edge(1 + offset, 0 + offset);
    target->sort_edges();
    check(offset);
}

BOOST_AUTO_TEST_CASE(sequential_subgraph_isomorphism_test1)
{
    auto query = create_query_graph();
    auto target = create_target_graph();
    config::slf_config cfg;
    cfg.set_max_log_results(1);
    sequential_subgraph_isomorphism solver(std::move(query), std::move(target),
                                           cfg);
    solver.run();
    BOOST_CHECK_EQUAL(solver.results_number(), 1);
}

BOOST_AUTO_TEST_CASE(parallel_subgraph_isomorphism_test1)
{
    auto query = create_query_graph();
    auto target = create_target_graph();
    config::slf_config cfg;
    cfg.set_max_log_results(1);
    parallel_subgraph_isomorphism solver(std::move(query), std::move(target),
                                         cfg);
    solver.run();
    BOOST_CHECK_EQUAL(solver.results_number(), 1);
}

void create_full_graph(graph_t& g, size_t node_number)
{
    g.set_node_number(node_number);
    for (size_t i = 0; i < node_number; i++)
    {
        for (size_t j = 0; j < node_number; j++)
        {
            g.add_edge(i, j, edge_label_t());
        }
    }
}
template <typename Solver_t, typename F>
size_t run_big_test(F&& f, int thread_number = 1)
{
    auto query = std::make_unique<graph_t>();
    auto target = std::make_unique<graph_t>();
    create_full_graph(*query, 100);
    create_full_graph(*target, 200);
    config::slf_config cfg;
    cfg.set_max_log_results(1);
    cfg.set_thread_number(thread_number);
    f(cfg);
    std::unique_ptr<slf::subgraph_isomorphism_base> solver =
        std::make_unique<Solver_t>(std::move(query), std::move(target), cfg);
    solver->run();
    return solver->results_number();
}

BOOST_AUTO_TEST_CASE(search_timeout_test1)
{
    auto f = [](config::slf_config& cfg)
    { return cfg.set_search_time_limitation_seconds(3); };
    BOOST_CHECK(run_big_test<sequential_subgraph_isomorphism>(f) > 1);
    BOOST_CHECK(run_big_test<parallel_subgraph_isomorphism>(f, 4) > 1);
}

BOOST_AUTO_TEST_CASE(search_results_limitation_test2)
{
    auto f = [](config::slf_config& cfg)
    { return cfg.set_search_results_limitation(3); };
    BOOST_CHECK_EQUAL(run_big_test<sequential_subgraph_isomorphism>(f), 3);
}
template <typename F_ret, typename P, typename C>
void task_manager_test_f(
    size_t thread_number_, P&& prepare_func,
    std::function<F_ret(size_t, atomic_bool&, task_manager&)>&& thread_func,
    C&& check_func)
{
    config::slf_config cfg;
    cfg.set_thread_number(thread_number_);
    auto query = std::make_unique<graph_t>();
    auto target = std::make_unique<graph_t>();
    auto solver_ = new parallel_subgraph_isomorphism(std::move(query),
                                                     std::move(target), cfg);
    task_manager tm(solver_);
    prepare_func(tm);
    atomic_bool run{false};
    std::vector<std::future<F_ret>> fus(thread_number_);
    for (size_t i = 0; i < thread_number_; i++)
        fus[i] = std::async(thread_func, i, std::ref(run), std::ref(tm));
    run = true;
    for (size_t i = 0; i < thread_number_; i++)
        fus[i].wait();
    check_func(fus);
}

BOOST_AUTO_TEST_CASE(task_manager_thread_exit)
{
    constexpr size_t thread_number_[4] = {2, 4, 8, 16};
    for (auto t : thread_number_)
    {
        auto f = [&](size_t, atomic_bool& run, task_manager& tm) -> bool
        {
            while (!run)
                ;
            std::shared_ptr<shared_tasks> ptr;
            if (tm.wait_shared_tasks_or_end(ptr))
                return false;
            else
                return true;
        };
        task_manager_test_f<bool>(
            t, [](task_manager&) {}, f,
            [](std::vector<std::future<bool>>& fus)
            {
                for (auto& var : fus)
                {
                    BOOST_CHECK_EQUAL(true, var.get());
                }
            });
    }
}

BOOST_AUTO_TEST_CASE(task_manager_thread_get_correct_task_and_quit)
{
    constexpr size_t thread_number_[4] = {2, 4, 8, 16};
    for (auto t : thread_number_)
    {
        std::unique_ptr<node_id_t[]> ptr_;
        size_t task_number = 4096;
        auto tasks_ptr_ = prepare_tasks<false>(task_number, true, ptr_);
        auto p = [&](task_manager& tm) { tm.share_tasks(*tasks_ptr_, {}); };
        auto f = [&](size_t, atomic_bool& run, task_manager& tm)
        {
            while (!run)
                ;
            std::vector<node_id_t> ret;
            std::shared_ptr<shared_tasks> ptr;
            node_id_t target_node_id_;
            if (tm.wait_shared_tasks_or_end(ptr))
            {
                while (ptr->get_task(target_node_id_))
                    ret.push_back(target_node_id_);
            }
            if (tm.wait_shared_tasks_or_end(ptr))
                ret.push_back(-1);
            return ret;
        };
        auto c = [&](std::vector<std::future<std::vector<node_id_t>>>& fus)
        {
            std::vector<node_id_t> res, exp(task_number);
            std::iota(exp.begin(), exp.end(), 0);
            for (auto& var : fus)
            {
                auto r = var.get();
                BOOST_REQUIRE(std::is_sorted(r.begin(), r.end()));
                res.insert(res.end(), r.begin(), r.end());
            }
            std::sort(res.begin(), res.end());
            BOOST_REQUIRE(res == exp);
        };
        task_manager_test_f<std::vector<node_id_t>>(t, p, f, c);
    }
}

BOOST_AUTO_TEST_CASE(task_manager_share_multi_times)
{
    constexpr size_t thread_number_[4] = {2, 4, 8, 16};
    for (auto t : thread_number_)
    {
        std::unique_ptr<node_id_t[]> ptr_;
        size_t task_number = 4096;
        auto tasks_ptr_ = prepare_tasks<false>(task_number, true, ptr_);
        auto p = [&](task_manager&) {};
        auto f = [&](size_t id, atomic_bool& run, task_manager& tm)
        {
            while (!run)
                ;
            if (id % 2)
            {
                std::vector<node_id_t> ts;
                std::shared_ptr<shared_tasks> ptr;
                node_id_t target_node_id_;
                bool ret = true;
                while (tm.wait_shared_tasks_or_end(ptr))
                {
                    ts.clear();
                    while (ptr->get_task(target_node_id_))
                        ts.push_back(target_node_id_);
                    if (std::is_sorted(ts.begin(), ts.end()) == false)
                        ret &= false;
                }
                return ret;
            }
            else
            {
                constexpr size_t share_times = 20;
                size_t tlen = 1000;
                size_t ben = share_times * tlen;
                auto ptrs = new std::unique_ptr<node_id_t[]>[share_times];
                for (size_t i = 0; i < share_times; i++)
                {
                    auto task_ptr_ =
                        prepare_tasks<false>(tlen, true, ptrs[i], ben);
                    ben -= tlen;
                    auto shared_tasks_ptr_ = tm.share_tasks(*task_ptr_, {});
                    while (tm.allow_sharing_flag() == false)
                        sleep_for_us(1);
                }
                std::shared_ptr<shared_tasks> ptr;
                while (tm.wait_shared_tasks_or_end(ptr))
                    sleep_for_us(1);
                return true;
            }
        };
        auto c = [&](std::vector<std::future<bool>>& fus)
        {
            for (auto& var : fus)
                BOOST_REQUIRE(true == var.get());
        };
        task_manager_test_f<bool>(t, p, f, c);
    }
}

BOOST_AUTO_TEST_CASE(task_manager_check_share_qualification)
{
    constexpr size_t thread_number_[4] = {2, 4, 8, 16};
    for (auto t : thread_number_)
    {
        auto p = [&](task_manager&) {};
        auto f = [&](size_t id, atomic_bool& run, task_manager& tm)
        {
            while (!run)
                ;
            if (id != t - 1)
            {
                std::shared_ptr<shared_tasks> ptr;
                if (tm.wait_shared_tasks_or_end(ptr))
                    return false;
                return true;
            }
            else
            {
                while (tm.allow_sharing_flag() == false)
                    sleep_for_us(0);
                size_t task_dep = 100;
                bool ret = true;
                for (size_t i = 0; i < task_dep; i++)
                {
                    if (tm.check_share_qualification(task_dep))
                        ret = false;
                }
                if (tm.check_share_qualification(task_dep) == false)
                    ret = false;
                for (size_t i = 0; i < t - 1; i++)
                {
                    if (tm.check_share_qualification(task_dep))
                        ret = false;
                }
                if (tm.check_share_qualification(task_dep) == false)
                    ret = false;
                std::shared_ptr<shared_tasks> ptr;
                if (tm.wait_shared_tasks_or_end(ptr))
                    ret = false;
                return ret;
            }
        };
        auto c = [&](std::vector<std::future<bool>>& fus)
        {
            for (auto& var : fus)
                BOOST_REQUIRE(true == var.get());
        };
        task_manager_test_f<bool>(t, p, f, c);
    }
}
} // namespace slf::test
