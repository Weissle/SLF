#pragma once
#include "graph/graph_base.h"
#include "predefined.h"
#include <atomic>
#include <boost/log/trivial.hpp>
#include <sstream>
#include <type_traits>
#include <vector>

namespace slf
{

template <bool is_multi_thread>
class results_taker
{
public:
    results_taker(size_t _print_count_max) : max_log_results_(_print_count_max)
    {
        if (_print_count_max == 0)
            BOOST_LOG_TRIVIAL(warning) << "results_taker::results_taker: The "
                                          "max_log_results is 0. All "
                                          "found mapping will be logged. In "
                                          "some cases, the log file's "
                                          "size will be very big.";
    }
    size_t operator()(const vector<graph::node_id_t>& mapping)
    {
        auto cnt = inc_counter() + 1;
        if (max_log_results_ == 0 || cnt <= max_log_results_)
        {
            log_result(mapping, cnt);
        }
        return cnt;
    }
    inline size_t max_log_results() const { return max_log_results_; }
    inline size_t result_number() const
    {
        if constexpr (is_multi_thread)
        {
            return counter_.load(std::memory_order_relaxed);
        }
        else
            return counter_;
    }
    using counter_t =
        std::conditional_t<is_multi_thread, std::atomic_size_t, size_t>;

	// this function will only used when the resutlts number is unlimited.
    inline void collect_thread_result(size_t val)
    {
		assert(is_multi_thread);
        counter_.fetch_add(val, std::memory_order_relaxed);
    }

private:
    void log_result(const vector<node_id_t>& mapping, size_t solution_id)
    {
        std::stringstream ss;
        ss << "answer_taker::log_result: Solution [" << solution_id << "]:";
        auto len = mapping.size();
        for (size_t i = 0; i < len; i++)
        {
            ss << " [" << i << "->" << mapping[i] << "]";
        }
        BOOST_LOG_TRIVIAL(info) << ss.str();
    }

    inline size_t inc_counter()
    {
        if constexpr (is_multi_thread)
        {
            return counter_.fetch_add(1, std::memory_order_relaxed);
        }
        else
            return counter_++;
    }

    size_t max_log_results_{0};
    counter_t counter_{0};
};

} // namespace slf
