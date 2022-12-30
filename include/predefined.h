#pragma once
#include "graph/graph.h"
namespace slf
{
using node_id_t = graph::node_id_t;
constexpr node_id_t error_node_id = graph::error_node_id;
using edge_label_t = graph::non_label_t;
using node_label_t = int;
using graph_t = graph::graph<edge_label_t, node_label_t>;
using source_edge_t = typename graph_t::source_edge_t;
using target_edge_t = typename graph_t::target_edge_t;
using node_t = typename graph_t::node_t;
} // namespace slf
