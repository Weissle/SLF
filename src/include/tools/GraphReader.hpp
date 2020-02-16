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
/*
fstream& openGraphFile(string graphPath,ios_base::openmode mode) {
	fstream f;
	f.open(graphPath.c_str(), mode);

}*/
template<typename F, typename S>
struct hash<pair<F, S> >
{
	size_t operator()(const pair<F, S>& p)const {
		auto hash1 = hash<F>()(p.first);
		auto hash2 = hash<S>()(p.second);
		return ((hash1 << 2) + 0x9e3779b9) ^ (hash2);
	}
};

template<class GraphType>
class LADReader {
	typedef typename GraphType::NodeType NodeType;

public:
	static GraphType* readGraph(string graphPath) {
		fstream f;

		f.open(graphPath.c_str(), ios_base::in);
		//		auto f = openGraphFile(graphPath, ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
		int nodeNum;
		f >> nodeNum;
		GraphType* graph = new GraphType(nodeNum);
		for (int i = 0; i < nodeNum; ++i) {
			int edgeNum;
			const int& source = i;
			f >> edgeNum;
			for (int j = 0; j < edgeNum; ++j) {
				int target;
				f >> target;
				graph->addEdge(source, target);
			}
		}


		f.close();
		return graph;
	}


};

template<class GraphType>
class ARGGraphNoLabel {
	typedef typename GraphType::NodeType NodeType;
public:
	static GraphType* readGraph(string graphPath) {

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
				graph->addEdge(source, target);
			}
		}


		fclose(f);
		return graph;
	}


};
/*
template<class GraphType>
class STFGraphNoLabel {
	typedef typename GraphType::NodeType NodeType;
public:
	static GraphType* readGraph(string graphPath) {
		fstream f;
		f.open(graphPath.c_str(), ios_base::in);
		//		auto f = openGraphFile(graphPath, ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
		int nodeNum=0;
		f >> nodeNum;

		GraphType *graph = new GraphType(nodeNum);


		for (int i = 0; i < edgeNum; ++i) {
			int source, target;
			f >> source >> target;
			graph->addEdge(source, target);
		}
		f.close();
		return graph;
	}

};*/
/*
template<class GraphType>
class STFGraphLabel {
	typedef typename GraphType::NodeType NodeType;
public:
	static GraphType* readGraph(string graphPath) {
		fstream f;
		f.open(graphPath.c_str(), ios_base::in);
		//		auto f = openGraphFile(graphPath, ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
		int nodeNum=0;
		f >> nodeNum;

		GraphType *graph = new GraphType(nodeNum);



		for (int i = 0; i < edgeNum; ++i) {
			size_t source, target;
			int label;
			f >> source >> target >> label;
			graph->addEdge(source, target, label);
		}


		f.close();
		return graph;
	}

};
*/
template<class GraphType>
class GRFGraphNoLabel {
public:
	static GraphType* readGraph(string graphPath) {
		fstream f;
		f.open(graphPath.c_str(), ios_base::in);
		//		auto f = openGraphFile(graphPath, ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
		int nodeNum = 0;
		f >> nodeNum;

		GraphType* graph = new GraphType(nodeNum);
		unordered_set< pair<size_t, size_t> > s;
		s.reserve(nodeNum * nodeNum);

		while (f.eof() == false) {
			size_t source = INT32_MAX, target = INT32_MAX;
			f >> source >> target;
			pair<size_t, size_t> p(source, target);

			if (IN_SET(s, p))continue;
			s.insert(p);
			graph->addEdge(source, target);
		}

		f.close();
		return graph;
	}
};

template<class GraphType>
class GRFGraphLabel {
public:
	static GraphType* readGraph(string graphPath) {
		fstream f;
		f.open(graphPath.c_str(), ios_base::in);
		if (f.is_open() == false) {
			cout << graphPath << " open fail" << endl;
			exit(0);
		}
		int nodeNum = 0;
		f >> nodeNum;

		GraphType* graph = new GraphType(nodeNum);

		for (auto i = 0; i < nodeNum; ++i) {
			size_t id;
			int label;
			f >> id >> label;
			graph->setNodeLabel(id, label - 1);
		}
		bool* pp = new bool[nodeNum]();
		while (f.eof() == false) {
			int edges = 0;
			f >> edges;
			unordered_set< pair<size_t, size_t> > s;
			s.reserve(calHashSuitableSize(edges));
			for (auto i = 0; i < edges; ++i) {

				size_t source = INT32_MAX, target = INT32_MAX;

				f >> source >> target;
				pair<size_t, size_t> p(source, target);
				if (IN_SET(s, p))continue;
				if (pp[source] == false) {
					graph->edgeVectorReserve(source, edges);
					pp[source] = true;
				}

				s.insert(p);
				graph->addEdge(source, target);
			}
		}

		f.close();
		return graph;
	}
};
