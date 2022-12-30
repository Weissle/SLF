#include "si/state.h"
#include "common.h"
#include <numeric>
#include <set>

namespace slf
{

compress_sub_state::compress_sub_state(size_t node_number_)
{
    in_depth_.resize(node_number_, max_depth);
    out_depth_.resize(node_number_, max_depth);
    match_sequence_rev_.resize(node_number_ + 1, error_node_id);
    node_sequence_.reserve(node_number_);
    match_sequence_.reserve(node_number_);
    cur_depth_ = 0;
}

std::optional<size_t> compress_sub_state::in_depth(node_id_t id,
                                                   size_t depth) const
{
    assert(depth <= node_sequence_.size());
    auto dep = in_depth_[id];
    if (dep <= depth)
        return dep;
    else
        return std::nullopt;
}
std::optional<size_t> compress_sub_state::out_depth(node_id_t id,
                                                    size_t depth) const
{
    assert(depth <= node_sequence_.size());
    auto dep = out_depth_[id];
    if (dep <= depth)
        return dep;
    else
        return std::nullopt;
}

bool compress_sub_state::in_set_in(node_id_t id, size_t depth) const
{
    assert(depth <= node_sequence_.size());
    return unmapped(id, depth) && in_depth(id, depth);
}

bool compress_sub_state::in_set_out(node_id_t id, size_t depth) const
{
    assert(depth <= node_sequence_.size());
    return unmapped(id, depth) && out_depth(id, depth);
}

bool compress_sub_state::unmapped(node_id_t id, size_t depth) const
{
    assert(depth <= node_sequence_.size());
    return match_sequence_rev_[id] > depth;
}

node_id_t compress_sub_state::match_id(size_t depth) const
{
    assert(depth);
    assert(match_sequence_rev_[match_sequence_[depth - 1]] == depth);
    assert(depth <= node_sequence_.size());
    return match_sequence_[depth - 1];
}

void compress_sub_state::add_node(const node_t& node)
{
    ++cur_depth_;
    node_sequence_.push_back(&node);
    match_sequence_.push_back(node.id());
    match_sequence_rev_[node.id()] = cur_depth_;
    for (const auto& e : node.source_edges())
    {
        auto source = e.source();
        if (unmapped(source, cur_depth_) && !in_depth(source, cur_depth_))
        {
            assert(in_depth_[source] == max_depth);
            in_depth_[source] = cur_depth_;
        }
    }
    for (const auto& e : node.target_edges())
    {
        auto target = e.target();
        if (unmapped(target, cur_depth_) && !out_depth(target, cur_depth_))
        {
            assert(out_depth_[target] == max_depth);
            out_depth_[target] = cur_depth_;
        }
    }
}

void compress_sub_state::remove_node()
{
    const auto& node = *node_sequence_.back();
    assert(node.id() == match_sequence_[cur_depth_ - 1]);
    match_sequence_.pop_back();
    match_sequence_rev_[node.id()] = error_node_id;
    for (const auto& e : node.source_edges())
    {
        auto source = e.source();
        auto in_dep = in_depth(source, cur_depth_);
        if (in_dep && in_dep.value() == cur_depth_)
        {
            assert(unmapped(node.id(), cur_depth_));
            in_depth_[source] = max_depth;
        }
    }
    for (const auto& e : node.target_edges())
    {
        auto target = e.target();
        auto out_dep = out_depth(target, cur_depth_);
        if (out_dep && out_dep.value() == cur_depth_)
        {
            assert(unmapped(node.id(), cur_depth_));
            out_depth_[target] = max_depth;
        }
    }
    node_sequence_.pop_back();
    --cur_depth_;
}

const vector<node_id_t>& compress_sub_state::match_sequence() const
{
    return match_sequence_;
}

query_complete_sub_state::query_complete_sub_state(
    const graph_t& g, const std::vector<node_id_t>& ms)
    : compress_sub_state(g.node_number())
{
    for (auto id : ms)
        add_node(g.get_node(id));
    auto n = g.node_number();
    source_infoes = std::make_unique<count_info[]>(n);
    target_infoes = std::make_unique<count_info[]>(n);
    for (size_t depth = 0; depth < n; ++depth)
    {
        auto& node = g.get_node(ms[depth]);
        auto f = [&](auto&& getter, auto& container, count_info& info)
        {
            info.in_ = info.out_ = info.not_ = info.both_ = info.mapped_ = 0;
            for (auto& e : container)
            {
                auto end_ = getter(e);
                bool unmapped_ = unmapped(end_, depth);
                if (unmapped_)
                {
                    bool i = in_set_in(end_, depth),
                         o = in_set_out(end_, depth);
                    bool b = i && o;
                    if (b)
                        ++info.both_;
                    else if (i)
                        ++info.in_;
                    else if (o)
                        ++info.out_;
                    else
                        ++info.not_;
                }
                else
                    ++info.mapped_;
            }
        };
        f([](const source_edge_t& e) { return e.source(); },
          node.source_edges(), source_infoes[depth]);
        f([](const target_edge_t& e) { return e.target(); },
          node.target_edges(), target_infoes[depth]);
    }
}

const query_complete_sub_state::count_info&
query_complete_sub_state::source_info([[maybe_unused]] const node_t& node,
                                      size_t depth) const
{
    assert(match_id(depth + 1) == node.id());
    return source_infoes[depth];
}

const query_complete_sub_state::count_info&
query_complete_sub_state::target_info([[maybe_unused]] const node_t& node,
                                      size_t depth) const
{
    assert(match_id(depth + 1) == node.id());
    return target_infoes[depth];
}

preset_tasks::preset_tasks(const graph_t& g)
{
    auto n = g.node_number();
    in_tasks_ = std::make_unique<std::vector<node_id_t>[]>(n);
    out_tasks_ = std::make_unique<std::vector<node_id_t>[]>(n);
    both_tasks_ = std::make_unique<std::vector<node_id_t>[]>(n);
    in_tasks_its_ = std::make_unique<task_info[]>(n);
    out_tasks_its_ = std::make_unique<task_info[]>(n);
    both_tasks_its_ = std::make_unique<task_info[]>(n);
    all_nodes_ = std::vector<node_id_t>(n);
    std::iota(all_nodes_.begin(), all_nodes_.end(), 0);
    all_nodes_its_ = task_info(all_nodes_.data(), all_nodes_.data() + n);
    std::vector<node_id_t> in(n), out(n), both(n);
    for (size_t i = 0; i < n; ++i)
    {
        in.clear();
        out.clear();
        both.clear();
        auto& node = g.get_node(i);
        for (auto& e : node.source_edges())
            in.push_back(e.source());
        for (auto& e : node.target_edges())
            out.push_back(e.target());
        sort_and_unique(in, false);
        sort_and_unique(out, false);
        auto it = std::set_intersection(in.begin(), in.end(), out.begin(),
                                        out.end(), both.begin());
        in_tasks_[i] = in;
        in_tasks_its_[i] = task_info(in_tasks_[i].data(),
                                     in_tasks_[i].data() + in_tasks_[i].size());
        out_tasks_[i] = out;
        out_tasks_its_[i] = task_info(
            out_tasks_[i].data(), out_tasks_[i].data() + out_tasks_[i].size());
        both_tasks_[i] = std::vector<node_id_t>(both.begin(), it);
        both_tasks_its_[i] =
            task_info(both_tasks_[i].data(),
                      both_tasks_[i].data() + both_tasks_[i].size());
    }
}

void preset_tasks::get_tasks(const std::optional<node_id_t>& in_match_id,
                             const std::optional<node_id_t>& out_match_id,
                             tasks<false>& t) const
{
    node_id_t in_id = in_match_id.value_or(error_node_id);
    node_id_t out_id = out_match_id.value_or(error_node_id);
    if (in_match_id && out_match_id)
    {
        if (in_id == out_id)
            set_tasks(t, both_tasks_its_[in_id]);
        else if (in_tasks_its_[in_id].size <= out_tasks_its_[out_id].size)
            set_tasks(t, in_tasks_its_[in_id]);
        else
            set_tasks(t, out_tasks_its_[out_id]);
    }
    else if (in_match_id)
        set_tasks(t, in_tasks_its_[in_id]);
    else if (out_match_id)
        set_tasks(t, out_tasks_its_[out_id]);
    else
    {
        assert(!in_match_id && !out_match_id);
        set_tasks(t, all_nodes_its_);
    }
}

std::shared_ptr<const preset_tasks>
preset_tasks::create_preset_tasks(const graph_t& g)
{
    auto ptr = new preset_tasks(g);
    return std::shared_ptr<const preset_tasks>(ptr);
}

state::state(const graph_t& query, const graph_t& target,
             std::shared_ptr<const query_complete_sub_state>&
                 _query_complete_sub_state_ptr,
             std::shared_ptr<const preset_tasks>& _preset_tasks_ptr)
    : query_(query), target_(target),
      query_complete_sub_state_ptr_(_query_complete_sub_state_ptr),
      preset_tasks_ptr_(_preset_tasks_ptr),
      target_sub_state_(target.node_number())
{
    auto query_node_number = query_.node_number();
    auto target_node_number = target_.node_number();
    mapping_.resize(query_node_number, error_node_id);
    mapping_rev_.resize(target_node_number, error_node_id);
}

void state::add_pair(node_id_t query_node_id, node_id_t target_node_id)
{
    assert(addable(query_node_id, target_node_id));
    assert(query_node_id ==
           query_complete_sub_state_ptr_->match_id(depth_ + 1));
    ++depth_;
    mapping_[query_node_id] = target_node_id;
    mapping_rev_[target_node_id] = query_node_id;
    target_sub_state_.add_node(target_.get_node(target_node_id));
}

bool state::addable(node_id_t query_node_id, node_id_t target_node_id) const
{
    assert(mapping_[query_node_id] == error_node_id);
    assert(query_node_id ==
           query_complete_sub_state_ptr_->match_id(depth_ + 1));
    if (mapping_rev_[target_node_id] != error_node_id)
        return false;
    auto& query_node = query_.get_node(query_node_id);
    auto& target_node = target_.get_node(target_node_id);
    if (query_node.label() != target_node.label() ||
        query_node.in_degree() > target_node.in_degree() ||
        query_node.out_degree() > target_node.out_degree())
        return false;
    if (query_node.get_self_loop() != target_node.get_self_loop())
        return false;
    size_t in{0}, out{0}, both{0}, not_{0};
    size_t source_mapped_num{0}, target_mapped_num{0};
    for (const auto& e : target_node.source_edges())
    {
        auto source = e.source();
        assert(source != target_node_id);
        auto unmapped = target_sub_state_.unmapped(source, depth_);
        if (unmapped)
        {
            auto i = target_sub_state_.in_set_in(source, depth_);
            auto o = target_sub_state_.in_set_out(source, depth_);
            auto b = i && o;
            if (b)
                ++both;
            else if (i)
                ++in;
            else if (o)
                ++out;
            else
                ++not_;
        }
        else
        {
            ++source_mapped_num;
            assert(mapping_rev_[source] != error_node_id);
            if (!query_.has_edge(mapping_rev_[source], query_node_id,
                                 e.label()))
                return false;
        }
    }

    const auto& source_count_info_ =
        query_complete_sub_state_ptr_->source_info(query_node, depth_);
    assert(source_count_info_.mapped_ >= source_mapped_num);

    if (in < source_count_info_.in_ || out < source_count_info_.out_ ||
        both < source_count_info_.both_ || not_ < source_count_info_.not_ ||
        source_mapped_num < source_count_info_.mapped_)
        return false;

    both = in = out = not_ = 0;
    for (const auto& e : target_node.target_edges())
    {
        auto target = e.target();
        assert(target != target_node_id);
        auto unmapped = target_sub_state_.unmapped(target, depth_);
        if (unmapped)
        {
            auto i = target_sub_state_.in_set_in(target, depth_);
            auto o = target_sub_state_.in_set_out(target, depth_);
            auto b = i && o;
            if (b)
                ++both;
            else if (i)
                ++in;
            else if (o)
                ++out;
            else
                ++not_;
        }
        else
        {
            ++target_mapped_num;
            assert(mapping_rev_[target] != error_node_id);
            if (!query_.has_edge(query_node_id, mapping_rev_[target],
                                 e.label()))
                return false;
        }
    }

    const auto& target_count_info_ =
        query_complete_sub_state_ptr_->target_info(query_node, depth_);

    assert(target_count_info_.mapped_ >= target_mapped_num);

    if (in < target_count_info_.in_ || out < target_count_info_.out_ ||
        both < target_count_info_.both_ || not_ < target_count_info_.not_ ||
        target_mapped_num < target_count_info_.mapped_)
        return false;

    return true;
}

void state::remove_pair()
{
    auto query_node_id = query_complete_sub_state_ptr_->match_id(depth_);
    auto target_node_id = target_sub_state_.match_id(depth_);
    assert(mapping_[query_node_id] == target_node_id &&
           mapping_rev_[target_node_id] == query_node_id);
    mapping_[query_node_id] = mapping_rev_[target_node_id] = error_node_id;
    target_sub_state_.remove_node();
    --depth_;
}

bool state::cover_query_graph() const { return depth_ == query_.node_number(); }

const std::vector<node_id_t>& state::mapping() const { return mapping_; }

const std::vector<node_id_t>& state::target_match_sequence() const
{
    return target_sub_state_.match_sequence();
}

bool state::add_pair_if_abbable(node_id_t query_node_id,
                                node_id_t target_node_id)
{
    if (addable(query_node_id, target_node_id))
    {
        add_pair(query_node_id, target_node_id);
        return true;
    }
    return false;
}

void state::get_tasks(node_id_t query_id, tasks<false>& task)
{
    auto in_dep = query_complete_sub_state_ptr_->in_depth(query_id, depth_);
    auto out_dep = query_complete_sub_state_ptr_->out_depth(query_id, depth_);
    std::optional<node_id_t> in_id, out_id;
    if (in_dep)
        in_id.emplace(target_sub_state_.match_id(in_dep.value()));
    if (out_dep)
        out_id.emplace(target_sub_state_.match_id(out_dep.value()));
    preset_tasks_ptr_->get_tasks(in_id, out_id, task);
}

} // namespace slf
