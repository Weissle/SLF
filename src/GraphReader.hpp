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

