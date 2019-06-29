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
	for (auto& node : graph.nodes()) f << node.id() << " " << node.getLabel() << endl;
	for (auto &node : graph.nodes()) {
		f << node.getOutEdgesNum() << endl;
		for (auto edge : node.getOutEdges()) {
			f << node.id() << " " << edge.getTargetNodeID() << endl;
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

	typedef size_t NodeIDType;
	typedef EdgeVF2< int> EdgeType;
	typedef NodeVF2<EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;


	auto *graph = GRFGraphLabel<GraphType>::readGraph(graphPath.c_str());
	graph->graphBuildFinish();
//	const auto *graph = STFGraphNoLabel<GraphType>::readGraph(graphPath.c_str());
	sg::SubgraphGenerator<GraphType> subgraphG(*graph, nodeNeed);

	subgraphG.run();
	const auto midGraph = subgraphG.getMid();
	writeMapping(midGraphPath, midGraph);
	const auto queryGraph = subgraphG.getSmallGraph();

	writeToFile(smallGraphPath, queryGraph);



}