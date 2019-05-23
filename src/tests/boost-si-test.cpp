//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <string>
#include <fstream>
using namespace boost;
using namespace std;


int main() {
//	boost::bid
	typedef adjacency_list<setS, vecS, bidirectionalS> graph_type;
	string queryGraphPath = "D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\smallGraph.graph";
	string targetGraphPath = "D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\bigGraph.graph";
	// Build graph1
	int num_vertices1 = 8;
	int num_vertices2 = 9;
/*	graph_type graph1(num_vertices1);
	add_edge(0, 6, graph1); add_edge(0, 7, graph1);
	add_edge(1, 5, graph1); add_edge(1, 7, graph1);
	add_edge(2, 4, graph1); add_edge(2, 5, graph1); add_edge(2, 6, graph1);
	add_edge(3, 4, graph1);

	// Build graph2

	graph_type graph2(num_vertices2);
	add_edge(0, 6, graph2); add_edge(0, 8, graph2);
	add_edge(1, 5, graph2); add_edge(1, 7, graph2);
	add_edge(2, 4, graph2); add_edge(2, 7, graph2); add_edge(2, 8, graph2);
	add_edge(3, 4, graph2); add_edge(3, 5, graph2); add_edge(3, 6, graph2);
*/
	

	fstream f;
	f.open(queryGraphPath.c_str(), ios::ios_base::in);

	f >> num_vertices1;
	graph_type graph1(num_vertices1);
	for (int i = 0; i < num_vertices1; ++i) {
		int edgeNum;
		const int &source = i;
		f >> edgeNum;
		for (int j = 0; j < edgeNum; ++j) {
			int target;
			f >> target;
			add_edge(source, target, graph1);
		}
	}
	f.close();
	f.open(targetGraphPath.c_str(), ios::ios_base::in);


	f >> num_vertices2;
	graph_type graph2(num_vertices2);
	for (int i = 0; i < num_vertices2; ++i) {
		int edgeNum;
		const int &source = i;
		f >> edgeNum;
		for (int j = 0; j < edgeNum; ++j) {
			int target;
			f >> target;
			add_edge(source, target, graph2);
		}
	}
	f.close();

	// Create callback to print mappings
	vf2_print_callback<graph_type, graph_type> callback(graph1, graph2);

	// Print out all subgraph isomorphism mappings between graph1 and graph2.
	// Vertices and edges are assumed to be always equivalent.
	vf2_subgraph_iso(graph1, graph2, callback);

	return 0;
}