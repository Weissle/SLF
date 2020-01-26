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
#include<unordered_set>
#include<iostream>
using namespace boost;
using namespace std;


int main() {
//	boost::bid
	
	string queryGraphPath = "D:\\data\\vsProject\\subgraph-isomorphism\\build\\small.graph";
	string targetGraphPath = "D:\\data\\vsProject\\subgraph-isomorphism\\build\\big.graph";
//	string orderPath = "D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\order.txt";
	// Build graph1
	int num_vertices1 = 0;
	int num_vertices2 = 0;

#define GRF_L
#ifdef ARG
	typedef adjacency_list<setS, vecS, bidirectionalS> graph_type;
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
	fclose(f);
#elif defined(LAD)
	typedef adjacency_list<setS, vecS, bidirectionalS> graph_type;
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
#elif defined(GRF_NL)

	fstream f;
	f.open(queryGraphPath.c_str(), ios::ios_base::in);
	f >> num_vertices1;
	graph_type graph1(num_vertices1);
	while(f.eof()==false) {
		size_t source, target;
		f >> source >> target;
		add_edge(source, target, graph1);
		
	}
	f.close();

	f.open(targetGraphPath.c_str(), ios::ios_base::in);

	f >> num_vertices2;
	graph_type graph2(num_vertices2);
	while (f.eof() == false) {
		size_t source, target;
		f >> source >> target;
		add_edge(source, target, graph2);

	}
	f.close();

#elif defined(GRF_L)
	typedef property<edge_name_t, int> edge_property;
	typedef property<vertex_name_t, int, property<vertex_index_t, int> > vertex_property;
	typedef adjacency_list<vecS, vecS, bidirectionalS, vertex_property, edge_property> graph_type;
	fstream f;
	f.open(queryGraphPath.c_str(), ios::ios_base::in);
	f >> num_vertices1;
	graph_type graph1;
	for (auto i = 0; i < num_vertices1; ++i) {
		int id, label;
		f >> id >> label;
		add_vertex(vertex_property(label), graph1);
	}
	while (f.eof() == false) {
		size_t edges=0;
		f >> edges;
		std::unordered_set<int> s;
		s.reserve(edges << 1);
		for (auto i = 0; i < edges; ++i) {
			size_t source, target;
			f >> source >> target;
			if (s.find(target) != s.end()) {
				cout << 1;
				continue;
			}
			else s.insert(target);
			add_edge(source, target, edge_property(0), graph1);
		}
	}
	f.close();

	f.open(targetGraphPath.c_str(), ios::ios_base::in);

	f >> num_vertices2;
	graph_type graph2;
	for (auto i = 0; i < num_vertices2; ++i) {
		int id, label;
		f >> id >> label;
		add_vertex(vertex_property(label), graph2);
	}
	while (f.eof() == false) {
		size_t edges=0;
		f >> edges;
		std::unordered_set<int> s;
		s.reserve(edges << 1);
		for (auto i = 0; i < edges; ++i) {
			size_t source, target;
			f >> source >> target;
			if (s.find(target) != s.end()) {
				continue;
				cout << 1;
			}
			else s.insert(target);
			add_edge(source, target, edge_property(0), graph2);
		}
	}
	f.close();

#endif
	// Create callback to print mappings
	cout << num_vertices1 << " " << num_vertices2 << endl;
	auto t1 = clock();
	vf2_print_callback<graph_type, graph_type> callback(graph1, graph2);
/*
	if (orderPath.empty() == false) {	
		vector<int> order;
		order.reserve(num_vertices1);
		f.open(orderPath.c_str(), ios::ios_base::in);
		while (f.eof() == false) {
			int temp;
			f >> temp;
			order.push_back(temp);
		}
		vf2_subgraph_iso(graph1, graph2, callback, order);
	}
	*/
	vf2_subgraph_iso(graph1, graph2, callback);
/*	vector<size_t> order = { 52 ,9 ,8 ,24 ,49 ,57 ,37 ,55 ,47 ,33 ,20 ,34 ,58 ,0 ,6 ,18 ,38 ,35 ,41 ,56 ,46 ,7 ,42 ,39 ,28 ,14 ,31 ,53 ,21 ,13 ,1 ,25 ,22 ,2 ,17 ,5 ,45 ,10 ,12 ,4 ,15 ,11 ,27 ,54 ,36 ,51 ,44 ,48 ,30 ,16 ,43 ,26 ,3 ,40 ,32 ,59 ,29 ,50 ,19 ,23 };
	vf2_subgraph_iso(graph1, graph2, callback,
		boost::get(vertex_index, graph1), get(vertex_index, graph2),
		order,
		always_equivalent(), always_equivalent());*/
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	// Print out all subgraph isomorphism mappings between graph1 and graph2.
	// Vertices and edges are assumed to be always equivalent.
	

	return 0;
}