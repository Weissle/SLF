#include "si/parallel_subgraph_isomorphism.h"
#include "tools/timer.h"
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

namespace slf
{

parallel_subgraph_isomorphism_thread::parallel_subgraph_isomorphism_thread(
    size_t id, parallel_subgraph_isomorphism* _solver,
    std::shared_ptr<shared_tasks> root_tasks_)
    : id_(id), solver_(_solver),
      state_(*_solver->query_graph_, *_solver->target_graph_,
             _solver->query_complete_state_sub_ptr_,
             _solver->preset_tasks_ptr_),
      tasks_(_solver->query_graph_->node_number()),
      nxt_shared_task_(std::move(root_tasks_))
{
}

void parallel_subgraph_isomorphism_thread::try_to_share()
{
    auto& tm = solver_->task_manager_;
    if (tm.allow_sharing_flag())
    {
        auto query_node_number = solver_->query_graph_->node_number();
        while (lowest_depth < query_node_number && tasks_[lowest_depth].empty())
            ++lowest_depth;
        if (lowest_depth == query_node_number ||
            tm.check_share_qualification(lowest_depth) == false)
            return;
        assert(nxt_shared_task_ == nullptr || nxt_shared_task_->empty());
        target_match_sequence_.clear();
        const auto& cur_tms = state_.target_match_sequence();
        target_match_sequence_.insert(target_match_sequence_.end(),
                                      cur_tms.begin(),
                                      cur_tms.begin() + lowest_depth);
		assert(tasks_[lowest_depth].size() > 0);
                nxt_shared_task_ = tm.share_tasks(tasks_[lowest_depth],
                                                  target_match_sequence_);
    }
}
void parallel_subgraph_isomorphism_thread::run()
{
    thread_search();
    BOOST_LOG_TRIVIAL(debug)
        << boost::format("parallel_subgraph_isomorphism_thread::run: Thread "
                         "[%1%] found [%2%] mappings, finished shared sub-task "
                         "[%3%], finished sub-task [%4%].") %
               id_ % solution_count_ % shared_subtasks_count_ % subtasks_count_;
}

void parallel_subgraph_isomorphism_thread::search()
{
    [[maybe_unused]] const auto& query = *solver_->query_graph_;
    const auto& match_sequence = solver_->match_sequence_;
    node_id_t query_node_id, target_node_id;
    size_t depth = state_.depth();
	const size_t start_depth = depth;
    state_.get_tasks(match_sequence[depth], tasks_[depth]);
    while (!solver_->is_end())
    {
        try_to_share();
        query_node_id = match_sequence[depth];
        while (tasks_[depth].get_task(target_node_id))
        {
            assert(depth == state_.depth());
            ++subtasks_count_;
            if (!state_.add_pair_if_abbable(query_node_id, target_node_id))
                continue;
            if (state_.cover_query_graph())
            {
                assert(depth + 1 == query.node_number());
                ++solution_count_;
                auto result_cnt = solver_->results_taker_(state_.mapping());
                if (solver_->search_results_limitation_ &&
                    result_cnt >= solver_->search_results_limitation_)
                {
                    solver_->stop();
                    return;
                }
                else
                    state_.remove_pair();
            }
            else
            {
                assert(depth + 1 < query.node_number());
                query_node_id = match_sequence[++depth];
                state_.get_tasks(query_node_id, tasks_[depth]);
            }
        }
        if (depth-- > start_depth)
            state_.remove_pair();
        else
            break;
    }
}

void parallel_subgraph_isomorphism_thread::thread_search()
{
    node_id_t query_node_id, target_node_id;
    std::shared_ptr<shared_tasks> st{nullptr};
    while (nxt_shared_task_ != nullptr ||
           solver_->task_manager_.wait_shared_tasks_or_end(nxt_shared_task_))
    {
        assert(nxt_shared_task_);
        st = std::move(nxt_shared_task_);
        prepare_state(st->target_match_sequence());
        query_node_id =
            solver_->match_sequence_[st->target_match_sequence().size()];
        while (st->get_task(target_node_id))
        {
            ++shared_subtasks_count_;
            if (state_.add_pair_if_abbable(query_node_id, target_node_id))
            {
                search();
                if (solver_->task_manager_.end_flag())
                    return;
                state_.remove_pair();
            }
        }
        st = nullptr;
    }
}

void parallel_subgraph_isomorphism_thread::prepare_state(
    const std::vector<node_id_t>& tms)
{
    auto& state_tms = state_.target_match_sequence();
    auto state_tms_size = state_tms.size();
    auto tms_size = tms.size();
    size_t idx{0};
    while (idx < tms_size && idx < state_tms_size && tms[idx] == state_tms[idx])
        ++idx;
    while (state_.depth() > idx)
        state_.remove_pair();
    const auto& qms = solver_->match_sequence_;
    for (auto i = idx; i < tms_size; ++i)
    {
        assert(state_.addable(qms[i], tms[i]));
        state_.add_pair(qms[i], tms[i]);
    }
	lowest_depth = state_.depth();
}

parallel_subgraph_isomorphism::parallel_subgraph_isomorphism(
    std::unique_ptr<graph_t> query, std::unique_ptr<graph_t> target,
    const slf::config::slf_config& slf_config)
    : subgraph_isomorphism_base(std::move(query), std::move(target),
                                slf_config.search_time_limitation_seconds(),
                                slf_config.search_results_limitation()),
      results_taker_(slf_config.max_log_results()),
      thread_number_(slf_config.thread_number()), task_manager_(this)

{
}

parallel_subgraph_isomorphism::parallel_subgraph_isomorphism(
    const std::string& query, const std::string& target,
    const slf::config::slf_config& slf_config)
    : subgraph_isomorphism_base(query, target, slf_config.graph_format(),
                                slf_config.search_time_limitation_seconds(),
                                slf_config.search_results_limitation()),
      results_taker_(slf_config.max_log_results()),
      thread_number_(slf_config.thread_number()), task_manager_(this)
{
}

void parallel_subgraph_isomorphism::search()
{
    std::vector<std::thread> threads(thread_number_);
    tasks<false> t;
    preset_tasks_ptr_->get_tasks(std::nullopt, std::nullopt, t);
    auto ptr = task_manager_.share_tasks(t, {});
    for (size_t id = 0; id < thread_number_; ++id)
    {
        threads[id] = std::thread(
            [](size_t id, parallel_subgraph_isomorphism* solver_,
               std::shared_ptr<shared_tasks> root_tasks_)
            {
                parallel_subgraph_isomorphism_thread thread_solver_(
                    id, solver_, root_tasks_);
                thread_solver_.run();
            },
            id, this, ptr);
    }
    ptr = nullptr;
    for (auto& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

} // namespace slf
