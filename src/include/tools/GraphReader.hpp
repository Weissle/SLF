#define _CRT_SECURE_NO_WARNING
#include"graph/Graph.hpp"
#include<string>
#include<fstream>
#include<iostream>
#include<typeinfo>
#include<unordered_set>
#include"si/si_marcos.h"
#include "IndexTurner.hpp"
#include<utility>
using namespace std;

struct hash_pair{
	template<typename F, typename S>
	size_t operator()(const pair<F, S>& p)const {
		auto hash1 = hash<F>()(p.first);
		auto hash2 = hash<S>()(p.second);
		return ((hash1 << 2) + 0x9e3779b9) ^ (hash2);
	}
};

using namespace wg;
template<typename EdgeLabel>
class GraphReader{
	using GraphType = GraphS<EdgeLabel>;
	void OpenFile(const string &graphPath,fstream &f){
		f.open(graphPath.c_str(), ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
	}
public:
	GraphReader()=default;
	// not sure is right !!
	GraphType* ReadFromLAD(string graphPath){
		fstream f;
		OpenFile(graphPath,f);
		int nodeNum;
		f >> nodeNum;
		GraphType* graph = new GraphType(nodeNum);
		for (int i = 0; i < nodeNum; ++i) {
			int edgeNum = -1;
			const int& source = i;
			f >> edgeNum;
			for (int j = 0; j < edgeNum; ++j) {
				int target = -1;
				f >> target;
				if(target == -1) break;
				graph->AddEdge(source, target);
			}
		}
		f.close();
		return graph;
	}

	// not sure is right !!
	GraphType* ReadFromARGNoLabel(string graphPath){
		FILE* f = fopen(graphPath.c_str(), "rb");
		assert(f != nullptr && "open file fail");
		int nodeNum;
		nodeNum = getwc(f);
		GraphType* graph = new GraphType(nodeNum);
		for (int i = 0; i < nodeNum; ++i) {
			int edgeNum = getwc(f);
			const int& source = i;

			for (int j = 0; j < edgeNum; ++j) {
				int target = getwc(f);
				graph->AddEdge(source, target);
			}
		}
		fclose(f);
		return graph;
	}	

	GraphType* ReadFromGRFNoLabel(string graphPath){
		fstream f;
		OpenFile(graphPath);
		int nodeNum = 0;
		f >> nodeNum;

		GraphType* graph = new GraphType(nodeNum);
		unordered_set< pair<size_t, size_t>,hash_pair > s;
		s.insert(error_pair);
		s.reserve(nodeNum * nodeNum);

		while (f.eof() == false) {
			size_t source = INT32_MAX, target = INT32_MAX;
			f >> source >> target;
			pair<size_t, size_t> p(source, target);

			if (s.count(p))continue;
			s.insert(p);
			graph->AddEdge(source, target);
		}

		f.close();
		return graph;
	}
	GraphType* ReadFromGRF(string graphPath) {
		fstream f;
		OpenFile(graphPath,f);
		int nodeNum = 0;
		f >> nodeNum;

		GraphType* graph = new GraphType(nodeNum);

		for (auto i = 0; i < nodeNum; ++i) {
			size_t id;
			int label;
			f >> id >> label;
			graph->SetNodeLabel(id, label);
		}
		while (f.eof() == false) {
			int edges = 0;
			f >> edges;
			unordered_set< pair<size_t, size_t> ,hash_pair> s;
			s.reserve(calHashSuitableSize(edges));
			for (auto i = 0; i < edges; ++i) {
				size_t source = UINT32_MAX, target = UINT32_MAX;
				f >> source >> target;
				if (source == UINT32_MAX || target == UINT32_MAX) continue;
				pair<size_t, size_t> p(source, target);
				if (s.count(p))continue;
				s.insert(p);
				graph->AddEdge(source, target);
			}
		}
		f.close();
		return graph;
	}

	/*
	   Each graph is described in a text file. If the graph has n vertices, then the file has n+1 lines:
		   The first line gives the number n of vertices.
		   The next n lines give, for each vertex, its number of successor nodes, followed by the list of its successor nodes.
	*/
	GraphType* ReadFromBN(string graphPath){
		fstream f;
		OpenFile(graphPath,f);
		int nodeNum = 0;
		f >> nodeNum;
		GraphType* graph = new GraphType(nodeNum);
		for (int i = 0; i < nodeNum; ++i){ 
			int edgesNum = 0;
			f>>edgesNum;
			for (int j = 0; j < edgesNum; ++j){ 
				int tar = -1;
				f >> tar;
				graph->AddEdge(i, tar);
			}
		}
		f.close();
		return graph;
	}
};
