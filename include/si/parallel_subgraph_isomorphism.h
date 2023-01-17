#pragma once
#include "si/results_taker.h"
#include "si/state.h"
#include "si/subgraph_isomorphism_base.h"
#include "si/task_manager.h"
#include "si/tasks.h"
namespace slf
{
class parallel_subgraph_isomorphism;
class parallel_subgraph_isomorphism_thread
{
    size_t id_;
    parallel_subgraph_isomorphism* solver_;
    state state_;
    std::vector<tasks<false>> tasks_;
	size_t lowest_depth{0};
	std::shared_ptr<shared_tasks> nxt_shared_task_{nullptr};
	size_t solution_count_{0};
    size_t shared_subtasks_count_{0};
    size_t subtasks_count_{0};

	// in memory temp variable
	std::vector<node_id_t> target_match_sequence_;

	void try_to_share();
    void search();
    void thread_search();
    void prepare_state(const std::vector<node_id_t>& tms);

public:
    parallel_subgraph_isomorphism_thread(
        size_t id, parallel_subgraph_isomorphism* _solver, std::shared_ptr<shared_tasks> root_tasks_);
    void run();
};

class parallel_subgraph_isomorphism : public subgraph_isomorphism_base
{
    results_taker<true> results_taker_;
    size_t thread_number_;
    task_manager task_manager_;

    void search() override;

public:
    parallel_subgraph_isomorphism(std::unique_ptr<graph_t> query,
                                  std::unique_ptr<graph_t> target,
                                  const slf::config::slf_config& slf_config);
    parallel_subgraph_isomorphism(const std::string& query,
                                  const std::string& target,
                                  const slf::config::slf_config& slf_config);
    size_t results_number() const override
    {
        return results_taker_.result_number();
    }
    size_t threads_number() const override { return thread_number_; }
    friend class parallel_subgraph_isomorphism_thread;
};

} // namespace slf
