#include "si/subgraph_isomorphism_base.h"
#include "graph/io.h"
#include "si/match_sequence_selector.h"
#include "tools/timer.h"
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <chrono>
#include <cstring>

namespace slf
{

subgraph_isomorphism_base::subgraph_isomorphism_base(
    std::unique_ptr<graph_t> query, std::unique_ptr<graph_t> target,
    long _search_time_limitation_seconds, size_t _results_limit)
    : search_results_limitation_(_results_limit), query_graph_(std::move(query)),
      target_graph_(std::move(target)),
      search_time_limitation_s_(_search_time_limitation_seconds)
{
}

subgraph_isomorphism_base::subgraph_isomorphism_base(
    const std::string& query, const std::string& target,
    const std::string& graph_format, long _search_time_limitation_seconds,
    size_t _results_limit)
    : search_results_limitation_(_results_limit), query_path_(query), target_path_(target),
      graph_format_(graph_format),
      search_time_limitation_s_(_search_time_limitation_seconds)
{
}

bool subgraph_isomorphism_base::read_graph(const char* name,
                                           std::unique_ptr<graph_t>& graph_ptr,
                                           std::string& path)
{
    assert(strcmp(name, "query") == 0 || strcmp(name, "target") == 0);
    if (graph_ptr)
        return true;
    double time_{0};
    {
        scope_timer st(time_);
        graph_ptr = graph::read_graph_from_file<graph_t>(path, graph_format_);
    }
    if (graph_ptr)
    {
        BOOST_LOG_TRIVIAL(debug)
            << boost::format("subgraph_isomorphism_base::read_graph: "
                             "succeed to read %1% graph from [%2%]. ") %
                   name % path;
        return true;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << boost::format("subgraph_isomorphism_base::read_graph: "
                             "fail to read %1% graph from [%2%].") %
                   name % path;
        return false;
    }
}

bool subgraph_isomorphism_base::read_query_graph()
{
    if (query_graph_)
        return true;
    return prepare_function_framework(
        [&]() { return read_graph("query", query_graph_, query_path_); },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::read_target_graph()
{
    if (target_graph_)
        return true;
    return prepare_function_framework(
        [&]() { return read_graph("target", target_graph_, target_path_); },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::compute_match_sequence()
{
    auto match_sequence_str = [&]()
    {
        std::stringstream ss;
        ss << "subgraph_isomorphism_base::compute_match_sequence: Result is [";
        for (auto id : match_sequence_)
        {
            ss << id << " ";
        }
        auto match_sequence_log = ss.str();
        match_sequence_log.back() = ']';
        return match_sequence_log;
    };
    return prepare_function_framework(
        [&]()
        {
            assert(query_graph_ && target_graph_);
            match_sequence_selector selector(*query_graph_, *target_graph_);
            match_sequence_ = selector.run();
            BOOST_LOG_TRIVIAL(trace) << match_sequence_str();
            return true;
        },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::compute_query_complete_sub_state()
{
    return prepare_function_framework(
        [&]()
        {
            query_complete_state_sub_ptr_ =
                std::shared_ptr<query_complete_sub_state>(
                    new query_complete_sub_state(*query_graph_,
                                                 match_sequence_));
            return true;
        },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::compute_preset_tasks()
{
    return prepare_function_framework(
        [&]()
        {
            assert(target_graph_);
            preset_tasks_ptr_ =
                preset_tasks::create_preset_tasks(*target_graph_);
            return true;
        },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::preprocess_query_graph()
{
    return prepare_function_framework(
        [&]()
        {
            assert(query_graph_);
            query_graph_->sort_edges();
            return true;
        },
        __FUNCTION__);
}

bool subgraph_isomorphism_base::preprocess_target_graph()
{
    return prepare_function_framework(
        [&]()
        {
            assert(target_graph_);
            target_graph_->sort_edges();
            return true;
        },
        __FUNCTION__);
}

void subgraph_isomorphism_base::start_time_consuming_guard()
{

    if (search_time_limitation_s_ == 0)
        return;
    time_consuming_guard_ = std::thread(
        [&]()
        {
            auto start_time = std::chrono::steady_clock::now();
            size_t sleep_us = 1000;
            size_t one_second_to_us = 1e6;
            while (!is_end())
            {
                sleep_for_us(sleep_us);
                auto time_dis = std::chrono::steady_clock::now() - start_time;
                auto time_dur =
                    std::chrono::duration_cast<std::chrono::seconds>(time_dis);
                if (time_dur.count() >= search_time_limitation_s_)
                {
                    stop();
                    BOOST_LOG_TRIVIAL(info)
                        << boost::format("subgraph_isomorphism_base::start_"
                                         "time_consuming_guard: Search timeout "
                                         "[%1%s]. Stop searching now.") %
                               search_time_limitation_s_;
                }
                sleep_us = (sleep_us >= one_second_to_us) ? one_second_to_us
                                                          : (sleep_us * 2);
            }
        });
}

subgraph_isomorphism_base::~subgraph_isomorphism_base()
{
    stop();
    if (time_consuming_guard_.joinable())
        time_consuming_guard_.join();
}

bool subgraph_isomorphism_base::is_end() const
{
    return end_.load(std::memory_order_relaxed);
}

void subgraph_isomorphism_base::stop()
{
    end_.store(true, std::memory_order_relaxed);
}

void subgraph_isomorphism_base::run()
{
    if (!read_query_graph() || !read_target_graph())
    {
        BOOST_LOG_TRIVIAL(error)
            << boost::format("subgraph_isomorphism_base::run: Failed to "
                             "read query or/and target graph. "
                             "This task is skip");
        return;
    }
    double total_compute_time{0};
    {
        scope_timer st1(total_compute_time);
        preprocess_query_graph();
        preprocess_target_graph();
        if (!compute_match_sequence() || !compute_query_complete_sub_state() ||
            !compute_preset_tasks())
            return;
        start_time_consuming_guard();
        search();
    }
    BOOST_LOG_TRIVIAL(info)
        << boost::format(
               "subgraph_isomorphism_base::run: Task (query graph:[%1%] target "
               "graph:[%2%]) is finished. Use [%3%] thread(s). "
               "Find mapping number [%4%]. Total Time cost: [%5%ms](w/o IO "
               "time).") %
               query_path_ % target_path_ % threads_number() %
               results_number() % (total_compute_time);
}

} // namespace slf
