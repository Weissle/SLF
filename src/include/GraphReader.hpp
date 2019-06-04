#define _CRT_SECURE_NO_WARNING
#include"Graph.hpp"
#include<string>
#include<fstream>
#include<iostream>
#include<typeinfo>

using namespace std;
/*
fstream& openGraphFile(string graphPath,ios_base::openmode mode) {
	fstream f;
	f.open(graphPath.c_str(), mode);
	
}*/

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
		vector<NodeType> nodeList;
		nodeList.reserve(nodeNum);
		for (int i = 0; i < nodeNum; ++i) {
			nodeList.push_back(NodeType(i));
		}
		GraphType *graph=new GraphType(nodeList);
		for (int i = 0; i < nodeNum; ++i) {
			int edgeNum;
			const int &source = i;
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

		FILE *f = fopen(graphPath.c_str(), "rb");
		assert(f != nullptr && "open file fail");

		int nodeNum; 
		nodeNum = getwc(f);

		vector<NodeType> nodeList;
		nodeList.reserve(nodeNum + 5);
		for (int i = 0; i < nodeNum; ++i) {
			nodeList.push_back(NodeType(i));
		}

		GraphType *graph = new GraphType(nodeList);
		for (int i = 0; i < nodeNum; ++i) {
			int edgeNum = getwc(f);
			const int &source = i;

			for (int j = 0; j < edgeNum; ++j) {
				int target = getwc(f);
				graph->addEdge(source, target);
			}
		}


		fclose(f);
		return graph;
	}


};

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
		int nodeNum;
		f >> nodeNum;
	
		vector<NodeType> nodeList;
		nodeList.reserve(nodeNum);
		int edgeNum;
		f >> edgeNum;
		for (int i = 0; i < nodeNum; ++i) {
			nodeList.push_back(NodeType(i));
		}
		GraphType *graph = new GraphType(nodeList);


		for (int i = 0; i < edgeNum; ++i) {
			int source, target;
			f >> source >> target;
			graph->addEdge(source, target);	
		}


		f.close();
		return graph;
	}

};

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
		int nodeNum;
		f >> nodeNum;

		vector<NodeType> nodeList;
		nodeList.reserve(nodeNum);
		int edgeNum;
		f >> edgeNum;
		for (int i = 0; i < nodeNum; ++i) {
			nodeList.push_back(NodeType(i));
		}
		GraphType *graph = new GraphType(nodeList);


		for (int i = 0; i < edgeNum; ++i) {
			int source, target;
			int label;
			f >> source >> target >> label;
			graph->addEdge(source, target, label);
		}


		f.close();
		return graph;
	}

};