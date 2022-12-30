#include "si/sequential_subgraph_isomorphism.h"
#include "si/state.h"
#include "tools/timer.h"
#include "tools/validation.h"

namespace slf
{

sequential_subgraph_isomorphism::sequential_subgraph_isomorphism(
    std::unique_ptr<graph_t> query, std::unique_ptr<graph_t> target,
    const slf::config::slf_config& slf_config)
    : subgraph_isomorphism_base(std::move(query), std::move(target),
                                slf_config.search_time_limitation_seconds(),
                                slf_config.search_results_limitation()),
      results_taker_(slf_config.max_log_results())
{
}

sequential_subgraph_isomorphism::sequential_subgraph_isomorphism(
    const std::string& query, const std::string& target,
    const slf::config::slf_config& slf_config)
    : subgraph_isomorphism_base(query, target, slf_config.graph_format(),
                                slf_config.search_time_limitation_seconds(),
                                slf_config.search_results_limitation()),
      results_taker_(slf_config.max_log_results())
{
}

void sequential_subgraph_isomorphism::search()
{
    std::vector<tasks_t> tasks_(query_graph_->node_number());
    size_t depth{0};
    static_assert(size_t{0} - 1 == std::numeric_limits<size_t>::max());
    state state_(*query_graph_, *target_graph_, query_complete_state_sub_ptr_, preset_tasks_ptr_);
    state_.get_tasks(match_sequence_[0], tasks_[0]);
    node_id_t query_node_id, target_node_id;

    while (!is_end())
    {
        query_node_id = match_sequence_[depth];
        while (tasks_[depth].get_task(target_node_id))
        {
			assert(depth == state_.depth());
            if (!state_.add_pair_if_abbable(query_node_id, target_node_id))
                continue;
            if (state_.cover_query_graph())
            {
                assert(depth + 1 == query_graph_->node_number());
                auto result_cnt = results_taker_(state_.mapping());
                if (search_results_limitation_ &&
                    result_cnt >= search_results_limitation_)
                    return;
                else
                    state_.remove_pair();
            }
            else
            {
                assert(depth + 1 < query_graph_->node_number());
                query_node_id = match_sequence_[++depth];
                state_.get_tasks(query_node_id, tasks_[depth]);
            }
        }
        if (depth--)
            state_.remove_pair();
        else
            break;
    }
}

} // namespace slf
