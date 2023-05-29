#pragma once
#include <optional>
#include "predefined.h"
#include "si/tasks.h"

namespace slf
{

class compress_sub_state
{
    inline static const auto max_depth = std::numeric_limits<size_t>::max();
    std::vector<size_t> in_depth_, out_depth_;
    std::vector<node_id_t> match_sequence_;
    // when this node is add to state, range
    // from 1 ~ query_graph_node_number;
    std::vector<size_t> match_sequence_rev_;
    std::vector<node_t const*> node_sequence_;
    size_t cur_depth_;

public:
    compress_sub_state(size_t node_number);
    std::optional<size_t> in_depth(node_id_t id, size_t depth) const;
    std::optional<size_t> out_depth(node_id_t id, size_t depth) const;
    bool in_set_in(node_id_t id, size_t depth) const;
    bool in_set_out(node_id_t id, size_t depth) const;
    bool unmapped(node_id_t id, size_t depth) const;
    node_id_t match_id(size_t depth) const;
    void add_node(const node_t& node);
    void remove_node();
    const vector<node_id_t>& match_sequence() const;
};

// TODO: ut
class query_complete_sub_state : public compress_sub_state
{
    struct count_info
    {
        size_t in_, out_, not_, both_, mapped_;
    };
    std::unique_ptr<count_info[]> source_infoes, target_infoes;

public:
    query_complete_sub_state(const graph_t& g,
                             const std::vector<node_id_t>& ms);
    const count_info& source_info(const node_t& node, size_t depth) const;
    const count_info& target_info(const node_t& node, size_t depth) const;
};

// TODO: ut
class preset_tasks
{
    struct task_info
    {
        const node_id_t *begin, *end;
        size_t size;
        task_info() : task_info(nullptr, nullptr) {}
        task_info(const node_id_t* _begin, const node_id_t* _end)
            : begin(_begin), end(_end), size(_end - _begin)
        {
            assert(_end >= _begin);
        }
    };
    std::unique_ptr<std::vector<node_id_t>[]> in_tasks_, out_tasks_,
        both_tasks_;
    std::unique_ptr<task_info[]> in_tasks_its_, out_tasks_its_, both_tasks_its_;
    std::vector<node_id_t> all_nodes_;
    task_info all_nodes_its_;
    inline void set_tasks(tasks<false>& t, const task_info& info) const
    {
        t.set_tasks(info.begin, info.end);
    }

public:
    preset_tasks(const graph_t& g);
    void get_tasks(const std::optional<node_id_t>& in_dep,
                   const std::optional<node_id_t>& out_dep,
                   tasks<false>& t) const;
    static std::shared_ptr<const preset_tasks>
    create_preset_tasks(const graph_t& g);
};

class state
{
public:
    state(const graph_t& query, const graph_t& target,
          std::shared_ptr<const query_complete_sub_state>&
              _query_complete_sub_state_ptr,
          std::shared_ptr<const preset_tasks>& _preset_tasks_ptr);
    void remove_pair();
    bool cover_query_graph() const;
    const std::vector<node_id_t>& mapping() const;
    const std::vector<node_id_t>& target_match_sequence() const;
    bool addable(node_id_t query_node_id, node_id_t target_node_id) const;
    void add_pair(node_id_t query_node_id, node_id_t target_node_id);
    bool add_pair_if_abbable(node_id_t query_node_id, node_id_t target_node_id);
    void get_tasks(node_id_t query_id, tasks<false>& task);

    size_t depth() const { return depth_; }

private:
    const graph_t &query_, &target_;
    size_t depth_{0}; // how many node in mapping
    std::shared_ptr<const query_complete_sub_state>
        query_complete_sub_state_ptr_;
    std::shared_ptr<const preset_tasks> preset_tasks_ptr_;
    compress_sub_state target_sub_state_;
    std::vector<node_id_t> mapping_, mapping_rev_;
};

} // namespace slf
