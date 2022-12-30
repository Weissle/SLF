#pragma once
#include "graph/graph_base.h"
#include <boost/core/noncopyable.hpp>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
namespace slf::graph
{

using node_id_t = uint32_t;
constexpr node_id_t error_node_id = std::numeric_limits<node_id_t>::max();

struct non_label_t : boost::noncopyable
{
    bool operator<(const non_label_t&) const { return false; }
    bool operator==(const non_label_t&) const { return true; }

};

inline constexpr non_label_t non_label;

template <typename T>
class label_base
{
public:
    using Label = T;
    label_base() = default;
    label_base(const T& label) : label_(label) {}
    const T& get() const { return label_; }
    void set(const T& label) { label_ = label; }
    static constexpr bool has_label() { return true; }

private:
    T label_{};
};

template <>
class label_base<non_label_t>
{

public:
    using Label = void;
    label_base() = default;
    label_base(const non_label_t&) {}
    const non_label_t& get() const { return non_label; }
    void set(const non_label_t&) {}
    static constexpr bool has_label() { return false; }
};

template <typename T>
using enable_if_label = std::enable_if_t<!std::is_same_v<T, non_label_t>, bool>;
template <typename T>
using enable_if_nonlabel =
    std::enable_if_t<std::is_same_v<T, non_label_t>, bool>;

} // namespace slf::graph
