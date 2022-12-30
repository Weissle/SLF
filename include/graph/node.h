#pragma once
#include <algorithm>
#include <boost/core/noncopyable.hpp>
#include <type_traits>
#include <vector>

#include "graph/edge.h"
#include "graph/graph_base.h"
#include "common.h"
namespace slf::graph
{

template <typename Label_>
class self_loop
{
public:
    using label_t = Label_;
    void add_edge(const label_t& label) { labels_.push_back(label); }
    void sort_edges() { sort_and_unique(labels_); }

    size_t edge_number() const { return labels_.size(); }
    bool has_edge(const label_t& label) const
    {
        return std::binary_search(labels_.begin(), labels_.end(), label);
    }

    bool operator!=(const self_loop<label_t>& rhs) const
    {
        return labels_ != rhs.labels_;
    }

private:
    std::vector<label_t> labels_;
};

template <>
class self_loop<non_label_t>
{
public:
    using Label = non_label_t;
    void add_edge(const non_label_t&) { has_ = true; }
    void sort_edges() { return; }
    size_t edge_number() const { return has_ ? 1 : 0; }
    bool has_edge(const non_label_t&) const { return has_; }
    bool operator!=(const self_loop<non_label_t>& rhs) const
    {
        return has_ != rhs.has_;
    }

private:
    bool has_{false};
};

template <typename edge_label_t_, typename node_label_t_>
class node : public label_base<node_label_t_>, boost::noncopyable
{
    using label_base_t = label_base<node_label_t_>;

public:
    using edge_label_t = edge_label_t_;
    using node_label_t = node_label_t_;
    using label_t = node_label_t;
    using source_edge_t = edge<edge_direction_enum::SOURCE, edge_label_t_>;
    using target_edge_t = edge<edge_direction_enum::TARGET, edge_label_t_>;
    using source_edge_container_t = std::vector<source_edge_t>;
    using target_edge_container_t = std::vector<target_edge_t>;
    static_assert(std::is_same_v<typename source_edge_t::Label,
                                 typename target_edge_t::Label>);

    static constexpr bool has_label()
    {
        return !std::is_same_v<node_label_t, non_label_t>;
    }
    static constexpr bool node_has_label() { return has_label(); }
    static constexpr bool edge_has_label()
    {
        return !std::is_same_v<edge_label_t, non_label_t>;
    }
    void sort_edges()
    {
        sort_and_unique(source_edges_);
        sort_and_unique(target_edges_);
        self_loop_.sort_edges();
    }

    size_t in_degree() const { return source_edges_.size(); }
    size_t out_degree() const { return target_edges_.size(); }
    const auto& source_edges() const { return source_edges_; }
    const auto& target_edges() const { return target_edges_; }

public:
    void add_edge(node_id_t source, node_id_t target, const edge_label_t& label)
    {
        if (source == id_ && target == id_)
            self_loop_.add_edge(label);
        else if (source == id_)
            target_edges_.emplace_back(target, label);
        else if (target == id_)
            source_edges_.emplace_back(source, label);
        else
            assert(false);
    }

    bool has_edge([[maybe_unused]] node_id_t source, node_id_t target,
                  const edge_label_t& label) const
    {
        assert(source == id_);
        if (target == id_)
        {
            return self_loop_.has_edge(label);
        }
        else
        {
            target_edge_t target_edge_;
            target_edge_.set_target(target);
            target_edge_.set_label(label);
            return std::binary_search(target_edges_.begin(),
                                      target_edges_.end(), target_edge_);
        }
    }

    bool has_edge([[maybe_unused]] node_id_t source, node_id_t target) const
    {
        static __thread target_edge_t target_edge_;
        static_assert(std::is_same_v<edge_label_t, non_label_t>);
        assert(source == id_);
        if (target == id_)
        {
            return self_loop_.has_edge(non_label);
        }
        else
        {
            target_edge_.set_target(target);
            return std::binary_search(target_edges_.begin(),
                                      target_edges_.end(), target_edge_);
        }
    }

    void set_label(const node_label_t& label) { label_base_t::set(label); }
    const node_label_t& label() const { return label_base_t::get(); }
    void set_id(const node_id_t& id) { id_ = id; }
    node_id_t id() const { return id_; }
    const self_loop<edge_label_t>& get_self_loop() const { return self_loop_; }

private:
    source_edge_container_t source_edges_;
    target_edge_container_t target_edges_;
    self_loop<edge_label_t> self_loop_;
    node_id_t id_{0xffffffff};
};

} // namespace slf::graph
