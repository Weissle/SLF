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
#include <time.h>
using namespace boost;
using namespace std;


int main() {
//	boost::bid
	typedef adjacency_list<setS, vecS, bidirectionalS> graph_type;
	string queryGraphPath = "D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\query.graph";
	string targetGraphPath = "D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\target.graph";
	// Build graph1
	int num_vertices1 = 8;
	int num_vertices2 = 9;

	
/*
	FILE *f = fopen(queryGraphPath.c_str(), "rb");
	assert(f != nullptr && "open file fail");
	num_vertices1 = getwc(f);
	graph_type graph1(num_vertices1);
	for (int i = 0; i < num_vertices1; ++i) {
		int edgeNum = fgetwc(f);
		const int &source = i;
		for (int j = 0; j < edgeNum; ++j) {
			int target = fgetwc(f);	
			add_edge(source, target, graph1);
		}
	}
	fclose(f);
	f = fopen(targetGraphPath.c_str(), "rb");


	num_vertices2 = fgetwc(f);
	graph_type graph2(num_vertices2);
	for (int i = 0; i < num_vertices2; ++i) {
		int edgeNum = fgetwc(f);
		const int &source = i;	
		for (int j = 0; j < edgeNum; ++j) {
			int target = fgetwc(f);	
			add_edge(source, target, graph2);
		}
	}
	fclose(f);*/
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
	cout << num_vertices1 << " " << num_vertices2 << endl;
	auto t1 = clock();
	vf2_print_callback<graph_type, graph_type> callback(graph1, graph2);
	vf2_subgraph_iso(graph1, graph2, callback);
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	// Print out all subgraph isomorphism mappings between graph1 and graph2.
	// Vertices and edges are assumed to be always equivalent.
	

	return 0;
}