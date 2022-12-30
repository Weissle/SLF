#pragma once
#include <algorithm>
#include <boost/core/noncopyable.hpp>
#include <cassert>
#include <memory>
#include <vector>

#include "graph/edge.h"
#include "graph/graph_base.h"
#include "graph/node.h"

using namespace std;
namespace slf::graph
{

template <typename edge_label_t_, typename node_label_t_>
class graph : boost::noncopyable
{
public:
    using source_edge_t = edge<edge_direction_enum::SOURCE, edge_label_t_>;
    using target_edge_t = edge<edge_direction_enum::TARGET, edge_label_t_>;
    using node_t = node<edge_label_t_, node_label_t_>;
    using edge_label_t = edge_label_t_;
    using node_label_t = node_label_t_;

private:
    std::unique_ptr<node_t[]> nodes_{nullptr};
    size_t node_number_{0};

public:
    graph() = default;
    void set_node_number(size_t node_number)
    {
        assert(node_number_ == 0);
        node_number_ = node_number;
        nodes_ = std::make_unique<node_t[]>(node_number_);
        for (size_t i = 0; i < node_number_; i++)
        {
            nodes_[i].set_id(i);
        }
    }
    size_t node_number() const { return node_number_; }
    static constexpr bool node_has_label() { return node_t::node_has_label(); }
    static constexpr bool edge_has_label() { return node_t::edge_has_label(); }

    void set_node_label(node_id_t node_id, const node_label_t& label)
    {
        static_assert(!std::is_same_v<node_label_t, non_label_t>);
        nodes_[node_id].set_label(label);
    }
    node_label_t node_label(node_id_t id) const { return nodes_[id].label(); }

    void sort_edges()
    {
        for (size_t i = 0; i < node_number_; i++)
            nodes_[i].sort_edges();
    }

    bool has_edge(node_id_t source, node_id_t target,
                  const edge_label_t& label) const
    {
        return nodes_[source].has_edge(source, target, label);
    }

    bool has_edge(node_id_t source, node_id_t target) const
    {
        static_assert(std::is_same_v<non_label_t, edge_label_t>);
        return nodes_[source].has_edge(source, target);
    }

    void add_edge(node_id_t source, node_id_t target, const edge_label_t& label)
    {
        nodes_[source].add_edge(source, target, label);
        nodes_[target].add_edge(source, target, label);
    }
    void add_edge(node_id_t source, node_id_t target)
    {
        static_assert(std::is_same_v<edge_label_t, non_label_t>);
        add_edge(source, target, non_label);
    }

    size_t node_in_degree(node_id_t id) const { return nodes_[id].in_degree(); }
    size_t node_out_degree(node_id_t id) const
    {
        return nodes_[id].out_degree();
    }
    const node_t& get_node(node_id_t id) const { return nodes_[id]; }

    const auto& get_node_source_edges(node_id_t id) const
    {
        return nodes_[id].source_edges();
    }
    const auto& get_node_target_edges(node_id_t id) const
    {
        return nodes_[id].target_edges();
    }
};
} // namespace slf::graph
