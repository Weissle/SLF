#include"GraphReader.hpp"
#include"SubgraphGenerator.hpp"
#include<string>
#include"argh.h"
using namespace std;

template<class GraphType>
void writeToFile(string fileName,GraphType graph) {
	ofstream f;
	f.flush();
	f.open(fileName.c_str(), ios::out);
	if (f.is_open() == false) {
		cout << fileName << " : open fail" << endl;
		return;
	}
	f << graph.size() << endl;
	for (auto &node : graph.getAllNodes()) {
		f << node.getOutEdgesNum() << " ";
		for (auto edge : node.getOutEdges()) {
			f << edge.getTargetNodeID() << " ";
		}
		f << endl;
	}
	f.close();
}

template<typename NodeIDType>
void writeMapping(const string fileName,const vector<NodeIDType> &nodes) {
	ofstream f;
	f.flush();
	f.open(fileName.c_str(), ios::out);
	if (f.is_open() == false) {
		cout << fileName << " : open fail" << endl;
		return;
	}
	for (int i = 0; i < nodes.size(); ++i) {
		f << i << " " << nodes[i] << endl;
	}
	f.close();
}
int main(int argc,char *argv[]) {
	
	argh::parser cmdl({ "-graph","-node" ,"-mg","-sg"});
	cmdl.parse(argc, argv);
	string graphPath, midGraphPath, smallGraphPath;
	int nodeNeed=0;
	cmdl({ "-graph"}) >> graphPath;
	cmdl({ "-node" }) >> nodeNeed;
	cmdl({ "-mg" }) >> midGraphPath;
	cmdl({ "-sg" }) >> smallGraphPath;
	assert(nodeNeed != 0);

	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;


	const auto *graph = LADReader<GraphType>::readGraph(graphPath.c_str());
//	const auto *graph = STFGraphNoLabel<GraphType>::readGraph(graphPath.c_str());
	sg::SubgraphGenerator<GraphType> subgraphG(*graph, nodeNeed);

	subgraphG.run();
	const auto midGraph = subgraphG.getMid();
	writeMapping(midGraphPath, midGraph);
	const auto queryGraph = subgraphG.getSmallGraph();

	writeToFile(smallGraphPath, queryGraph);



}