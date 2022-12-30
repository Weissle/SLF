#pragma once
#include <assert.h>
#include <tuple>
#include "graph/graph_base.h"

namespace slf::graph
{

// Which end does this edge store.
enum edge_direction_enum : char
{
    TARGET = 'T',
    SOURCE = 'S'
};

template <edge_direction_enum direction, typename Label_>
class edge : public label_base<Label_>
{
    using self_t = edge<direction, Label_>;
    using label_base_t = label_base<Label_>;

protected:
    node_id_t get_end() const { return end_; }

public:
    // common
    using Label = Label_;
    edge() = default;

    edge(node_id_t _end, const Label& label) : label_base_t(label), end_(_end) {}
    edge(node_id_t _end) : end_(_end) {}
    static constexpr bool has_label() { return !std::is_same_v<Label_, void>; }
    edge_direction_enum edge_direction() const { return direction; }

    node_id_t source() const
    {
        static_assert(direction == edge_direction_enum::SOURCE);
        return end_;
    }
    node_id_t target() const
    {
        static_assert(direction == edge_direction_enum::TARGET);
        return end_;
    }

    void set_source(node_id_t source_)
    {
        static_assert(direction == edge_direction_enum::SOURCE);
        end_ = source_;
    }
    void set_target(node_id_t target_)
    {
        static_assert(direction == edge_direction_enum::TARGET);
        end_ = target_;
    }

    const Label& label() const { return label_base_t::get(); }
    void set_label(const Label& label) { label_base_t::set(label); }

    bool operator<(const self_t& e) const
    {
        return std::tie(end_, label_base_t::get()) <
               std::tie(e.end_, e.label());
    }
    bool operator==(const self_t& e) const
    {
        return std::tie(end_, label_base_t::get()) ==
               std::tie(e.end_, e.label());
    }

private:
    node_id_t end_;
};

} // namespace slf::graph
