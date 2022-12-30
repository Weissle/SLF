#include "common.h"
#include <boost/log/trivial.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
namespace slf::graph
{
template <typename graph_t, typename stream_t>
std::unique_ptr<graph_t> read_grf_graph(stream_t& stream)
{
    constexpr bool has_edge_label = graph_t::edge_has_label();
    constexpr bool has_node_label = graph_t::node_has_label();
    using node_label_t = typename graph_t::node_label_t;
    using edge_label_t = typename graph_t::edge_label_t;
    auto graph_ = std::make_unique<graph_t>();
    size_t node_number_;
    stream >> node_number_;
    graph_->set_node_number(node_number_);
    if constexpr (has_node_label)
    {
        size_t id_;
        node_label_t label_;
        for (size_t i = 0; i < node_number_; i++)
        {
            stream >> id_ >> label_;
            assert(i == id_);
            graph_->set_node_label(id_, label_);
        }
    }
    for (size_t i = 0; i < node_number_; i++)
    {
        size_t edge_number_;
        stream >> edge_number_;
        for (size_t j = 0; j < edge_number_; j++)
        {
            node_id_t source{error_node_id}, target{error_node_id};
            stream >> source >> target;
            assert(source != error_node_id && target != error_node_id);
            assert(i == source);
            if constexpr (has_edge_label)
            {
                edge_label_t label_;
                stream >> label_;
                graph_->add_edge(source, target, label_);
            }
            else
            {
                graph_->add_edge(source, target);
            }
        }
    }
    return graph_;
}

template <typename graph_t>
std::unique_ptr<graph_t> read_grf_graph_from_file(const std::string& path)
{
    std::ifstream ifstream_;
    if (!open_fstream(path, ifstream_))
    {
        return nullptr;
    }
    else
        return read_grf_graph<graph_t>(ifstream_);
}

template <typename graph_t>
std::unique_ptr<graph_t> read_graph_from_file(const std::string& path,
                                              const std::string& graph_format)
{
    if (graph_format == "grf")
    {
        return read_grf_graph_from_file<graph_t>(path);
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "read_graph_from_file: graph_format ["
                                 << graph_format << "] is not supported";
        return nullptr;
    }
}

} // namespace slf::graph
