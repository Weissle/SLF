#define BOOST_TEST_MODULE graph_unittest
#include "graph/edge.h"
#include "graph/graph.h"
#include "graph/graph_base.h"
#include "graph/io.h"
#include "graph/node.h"
#include <boost/test/included/unit_test.hpp>
#include <sstream>
namespace slf::graph::test
{

BOOST_AUTO_TEST_CASE(NonLabel_test)
{
    non_label_t label1, label2;
    BOOST_CHECK(!(label1 < label2));
    BOOST_CHECK(!(label2 < label1));
    BOOST_CHECK(label2 == label1);
}

BOOST_AUTO_TEST_CASE(LabelBase_normal_test)
{
    label_base<int> label(3);
    BOOST_CHECK(label.has_label());
    BOOST_CHECK_EQUAL(label.get(), 3);
    label.set(4);
    BOOST_CHECK_EQUAL(label.get(), 4);
}

BOOST_AUTO_TEST_CASE(LabelBase_nonlabel_test)
{
    label_base<non_label_t> label;
    BOOST_CHECK(!label.has_label());
}

BOOST_AUTO_TEST_CASE(Edge_nonlabel_test)
{
    edge<edge_direction_enum::SOURCE, non_label_t> e1(1), e2(2);
    edge<edge_direction_enum::TARGET, non_label_t> e3(3), e4(4);
    BOOST_CHECK(e1 < e2);
    BOOST_CHECK(e3 < e4);
    e1.set_source(2);
    BOOST_CHECK_EQUAL(e1.source(), 2);
    BOOST_CHECK(e1 == e2);
    e3.set_target(4);
    BOOST_CHECK_EQUAL(e3.target(), 4);
    BOOST_CHECK(e3 == e4);
}

BOOST_AUTO_TEST_CASE(Edge_label_test)
{
    edge<edge_direction_enum::SOURCE, int> e1(1, 1), e2(2, 1);
    edge<edge_direction_enum::TARGET, int> e3(3, 1), e4(4, 1);
    BOOST_CHECK(e1 < e2);
    BOOST_CHECK(e3 < e4);
    e1.set_source(2);
    BOOST_CHECK_EQUAL(e1.source(), 2);
    BOOST_CHECK(e1 == e2);
    e3.set_target(4);
    BOOST_CHECK_EQUAL(e3.target(), 4);
    BOOST_CHECK(e3 == e4);
    e1.set_label(2);
    BOOST_CHECK(e2 < e1);
}

BOOST_AUTO_TEST_CASE(SelfLoop_nonlabel_test)
{
    self_loop<non_label_t> sl, sl2;
    BOOST_CHECK_EQUAL(sl.edge_number(), 0);
    BOOST_CHECK(false == (sl != sl2));
    BOOST_CHECK(!sl.has_edge(non_label));
    sl.add_edge(non_label);
    BOOST_CHECK_EQUAL(sl.edge_number(), 1);
    BOOST_CHECK(sl.has_edge(non_label));
    BOOST_CHECK(sl != sl2);
    for (size_t i = 0; i < 10; i++)
    {
        sl.add_edge(non_label);
        BOOST_CHECK(sl.has_edge(non_label));
    }
    sl2.add_edge(non_label);
    BOOST_CHECK(false == (sl != sl2));
}

BOOST_AUTO_TEST_CASE(SelfLoop_normal_test)
{
    self_loop<int> sl, sl2;
    BOOST_CHECK(false == (sl != sl2));
    auto add_edge_f = [](auto &self_loop_)
    {
        for (size_t i = 0; i < 10; i++)
        {
            BOOST_CHECK(!self_loop_.has_edge(i * 10));
            self_loop_.add_edge(i * 10);
            BOOST_CHECK_EQUAL(self_loop_.edge_number(), i + 1);
            BOOST_CHECK(self_loop_.has_edge(i * 10));
        }
        BOOST_CHECK(!self_loop_.has_edge(1));
        self_loop_.add_edge(1);
        self_loop_.sort_edges();
        BOOST_CHECK(self_loop_.has_edge(1));
    };
    add_edge_f(sl);
    BOOST_CHECK(sl != sl2);
    add_edge_f(sl2);
    BOOST_CHECK(false == (sl != sl2));
}

BOOST_AUTO_TEST_CASE(node_nonlabel_test)
{
    node<non_label_t, non_label_t> node1, node2;
    BOOST_CHECK(node1.label() == node2.label());
    node1.set_id(1);
    BOOST_CHECK_EQUAL(node1.id(), 1);
}

BOOST_AUTO_TEST_CASE(node_label_test)
{
    node<non_label_t, int> node1, node2;
    node1.set_label(1);
    node2.set_label(2);
    BOOST_CHECK(node1.label() != node2.label());
    node2.set_label(1);
    BOOST_CHECK_EQUAL(node1.label(), node2.label());
}

BOOST_AUTO_TEST_CASE(node_common_test)
{
    node<int, int> node1;
    node1.set_id(1);
    node_id_t sources[5] = {0, 1, 2, 3, 4};
    node_id_t targets[5] = {0, 1, 2, 3, 4};
    int labels[2] = {1, 2};
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            BOOST_CHECK(!node1.has_edge(1, targets[i], labels[j]));
            node1.add_edge(1, targets[i], labels[j]);
            node1.add_edge(sources[i], 1, labels[j]);
            // test sort and filter same edge.
            node1.add_edge(1, targets[i], labels[j]);
            node1.add_edge(sources[i], 1, labels[j]);
        }
    }
    BOOST_CHECK_EQUAL(node1.in_degree(), 8 * 2);
    BOOST_CHECK_EQUAL(node1.out_degree(), 8 * 2);
    node1.sort_edges();
    BOOST_CHECK_EQUAL(node1.in_degree(), 8);
    BOOST_CHECK_EQUAL(node1.out_degree(), 8);
    BOOST_CHECK(std::is_sorted(node1.source_edges().begin(),
                               node1.source_edges().end()));
    BOOST_CHECK(std::is_sorted(node1.target_edges().begin(),
                               node1.target_edges().end()));
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            BOOST_CHECK(node1.has_edge(1, targets[i], labels[j]));
        }
    }
}

BOOST_AUTO_TEST_CASE(graph_common_test1)
{
    constexpr int node_number = 10;
    graph<non_label_t, non_label_t> g;
    g.set_node_number(node_number);
    BOOST_CHECK_EQUAL(g.node_has_label(), false);
    BOOST_CHECK_EQUAL(g.edge_has_label(), false);
    BOOST_CHECK_EQUAL(g.node_number(), node_number);
    for (size_t i = 0; i < node_number; i++)
    {
        BOOST_CHECK_EQUAL(g.node_in_degree(i), 0);
        BOOST_CHECK_EQUAL(g.node_out_degree(i), 0);
    }
    for (size_t i = 0; i < node_number; i++)
    {
        if (i + 1 < node_number)
            g.add_edge(i, i + 1);
        if (i > 0)
            g.add_edge(i, i - 1);
    }
    g.sort_edges();
    for (size_t i = 0; i < node_number; i++)
    {
        int exp_in_degree = (i == 0 || i == node_number - 1) ? 1 : 2;
        int exp_out_degree = (i == 0 || i == node_number - 1) ? 1 : 2;
        BOOST_CHECK_EQUAL(g.node_in_degree(i), exp_in_degree);
        BOOST_CHECK_EQUAL(g.node_out_degree(i), exp_out_degree);
        if (i + 1 < node_number)
        {
            BOOST_CHECK(g.has_edge(i, i + 1, non_label));
        }
        if (i > 0)
        {
            BOOST_CHECK(g.has_edge(i, i - 1, non_label));
        }
    }
    g.add_edge(1, 1);
    g.sort_edges();
    BOOST_CHECK(g.has_edge(1, 1, non_label));
}

BOOST_AUTO_TEST_CASE(graph_common_test2)
{
    constexpr int node_number = 10;
    graph<int, int> g;
    g.set_node_number(node_number);
    BOOST_CHECK_EQUAL(g.node_has_label(), true);
    BOOST_CHECK_EQUAL(g.edge_has_label(), true);
    BOOST_CHECK_EQUAL(g.node_number(), node_number);
    for (size_t i = 0; i < node_number; i++)
    {
        g.set_node_label(i, i ^ 1);
        BOOST_CHECK_EQUAL(g.node_label(i), i ^ 1);
        BOOST_CHECK_EQUAL(g.node_in_degree(i), 0);
        BOOST_CHECK_EQUAL(g.node_out_degree(i), 0);
    }
    for (size_t i = 0; i < node_number; i++)
    {
        if (i + 1 < node_number)
            g.add_edge(i, i + 1, 5);
        if (i > 0)
            g.add_edge(i, i - 1, 4);
    }
    g.sort_edges();
    for (size_t i = 0; i < node_number; i++)
    {
        int exp_in_degree = (i == 0 || i == node_number - 1) ? 1 : 2;
        int exp_out_degree = (i == 0 || i == node_number - 1) ? 1 : 2;
        BOOST_CHECK_EQUAL(g.node_in_degree(i), exp_in_degree);
        BOOST_CHECK_EQUAL(g.node_out_degree(i), exp_out_degree);
        if (i + 1 < node_number)
        {
            BOOST_CHECK(g.has_edge(i, i + 1, 5));
            BOOST_CHECK(!g.has_edge(i, i + 1, 4));
        }
        if (i > 0)
        {
            BOOST_CHECK(g.has_edge(i, i - 1, 4));
            BOOST_CHECK(!g.has_edge(i, i - 1, 5));
        }
    }
    g.add_edge(1, 1, 3);
    g.sort_edges();
    BOOST_CHECK(g.has_edge(1, 1, 3));
}

BOOST_AUTO_TEST_CASE(grf_graph_read1)
{
    std::string graph_str = R"(
	3 
	0 1 
	1 0 
	2 1 
	1 
	0 2 
	2 
	1 0 
	1 1 
	1 
	2 1 )";
    std::stringstream ss(graph_str);
    auto g = read_grf_graph<graph<non_label_t, int>>(ss);
    auto& graph = *g;
    graph.sort_edges();
    BOOST_CHECK_EQUAL(graph.node_number(), 3);
    BOOST_CHECK_EQUAL(graph.node_label(0), 1);
    BOOST_CHECK_EQUAL(graph.node_label(1), 0);
    BOOST_CHECK_EQUAL(graph.node_label(2), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(0), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(0), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(1), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(1), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(2), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(2), 1);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 2),false);
    BOOST_CHECK_EQUAL(graph.has_edge(2, 1),true);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 1),true);
    BOOST_CHECK_EQUAL(graph.has_edge(0, 2),true);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 0),true);
}

BOOST_AUTO_TEST_CASE(grf_graph_read2)
{
    std::string graph_str = R"(
	3 
	0 1 
	1 0 
	2 1 
	1 
	0 2 0
	2 
	1 0 1
	1 1 1
	1 
	2 1 2
	)";
    std::stringstream ss(graph_str);
    auto g = read_grf_graph<graph<int, int>>(ss);
    auto& graph = *g;
    graph.sort_edges();
    BOOST_CHECK_EQUAL(graph.node_number(), 3);
    BOOST_CHECK_EQUAL(graph.node_label(0), 1);
    BOOST_CHECK_EQUAL(graph.node_label(1), 0);
    BOOST_CHECK_EQUAL(graph.node_label(2), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(0), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(0), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(1), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(1), 1);
    BOOST_CHECK_EQUAL(graph.node_in_degree(2), 1);
    BOOST_CHECK_EQUAL(graph.node_out_degree(2), 1);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 2,1),false);
    BOOST_CHECK_EQUAL(graph.has_edge(2, 1,2),true);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 1,1),true);
    BOOST_CHECK_EQUAL(graph.has_edge(0, 2,0),true);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 0,1),true);
    BOOST_CHECK_EQUAL(graph.has_edge(2, 1,1),false);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 1,2),false);
    BOOST_CHECK_EQUAL(graph.has_edge(0, 2,1),false);
    BOOST_CHECK_EQUAL(graph.has_edge(1, 0,0),false);
}

} // namespace slf::graph::test
