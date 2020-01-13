#include"GraphReader.hpp"
#include"SubgraphGenerator.hpp"
#include<string>
#include<RandomGenerator.hpp>
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
	for (auto& node : graph.nodes()) f << node.id() << " " << node.getLabel()+1 << endl;
	for (auto &node : graph.nodes()) {
		f << node.getOutEdgesNum() << endl;
		for (auto edge : node.getOutEdges()) {
			f << node.id() << " " << edge.getTargetNodeID() << endl;
		}
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
	
	argh::parser cmdl({ "-graph","-small-node" ,"-mg","-sg","big-node","-edge-avg","-labels","-bg","-randomModel","-variance","-labelTypes" });
	cmdl.parse(argc, argv);
	string graphPath, midGraphPath, smallGraphPath, bigGraphPath, randomModel = "normal";
	int sNeed = 0, bNeed = 0, labelTypes = 5;
	double variance = 1;
	double edgeavg = 1;
	cmdl({ "-graph"}) >> graphPath;
	cmdl({ "-labelTypes" }) >> labelTypes;
	cmdl({ "-small-node" }) >> sNeed;
	cmdl({ "-edge-avg" }) >> edgeavg;
	cmdl({ "-big-node" }) >> bNeed;
	cmdl({ "-mg" }) >> midGraphPath;
	cmdl({ "-bg" }) >> bigGraphPath;
	cmdl({ "-sg" }) >> smallGraphPath;
	cmdl({ "-random-model" }) >> randomModel;
	cmdl({ "-variance" }) >> variance;
	assert(sNeed != 0);
	assert((graphPath.empty() ^ bNeed == 0) );
	typedef size_t NodeIDType;
	typedef EdgeVF2< int> EdgeType;
	typedef NodeVF2<EdgeType> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;

	srand(uint32_t(time(NULL)));
	rg::RandomGenerator* randomer = nullptr;
	for_each(randomModel.begin(), randomModel.end(), [](char& c) {
		c = tolower(c);
		});
	if (randomModel == "normal") {
		randomer = new rg::NormalRandomGenerator(edgeavg, variance);
	}
	else if (randomModel == "mean") {
		int t = (int)sqrt(12 * variance);
		const double& m = edgeavg;
		int Max = max(0, (int)((double)(2*m + t) / 2));
		int Min = max(0, (int)(Max - t));
		randomer = new rg::MeanlyRandomGenerator(Min, Max);
		
	}
	GraphType* graph=nullptr;
	if (graphPath.empty()) {
		assert(edgeavg > 0 && edgeavg > 1E-10);
		assert(bNeed > sNeed);
		vector<NodeType> nodes(bNeed);
		LOOP(i, 0, labelTypes) {
			nodes[i] = NodeType(i, i);
			nodes[i].reserve(edgeavg);
		}
		LOOP(i, labelTypes, bNeed) {
			nodes[i] = NodeType(i, rand() % labelTypes);
			nodes[i].reserve(edgeavg);
		}
		graph =new GraphType(nodes);		
		vector<NodeIDType> in(bNeed), no(bNeed);
		int inP=1, noP = bNeed-1;
		LOOP(i, 0, bNeed-1) no[i] = i;
		in[0] = bNeed - 1;
		LOOP(i, 0, bNeed - 1) {
			auto one = rand() % inP;
			auto two = rand() % noP;
			no[two] = --noP;
			in[inP++] = no[two];
			if (rand() % 2) graph->addEdge(in[one], no[two]);
			else graph->addEdge(no[two], in[one]);
		}
		LOOP(i, 0, bNeed) {
			int edgeNum = min(max(0,(int)randomer->getOne()),bNeed-2);
			rg::NoRepeatIntRandomGenerator irg(bNeed);
		//	for (const auto& edge : graph->getNode(i).getOutEdges()) s.insert(edge.getTargetNodeID());
			auto outEdges = graph->getNode(i).getOutEdges();
			assert(outEdges.size() < 2 && "outEdge too much");
			const NodeIDType exclude = (outEdges.size() == 0) ? -1 : outEdges[0].getTargetNodeID();
			LOOP(j, 0, edgeNum) {
				NodeIDType to = irg.getOne();
				if (i == to || i==exclude) {
					--j;
					continue;
				}
				graph->addEdge(i, to);
			}
		}	
	}
	else graph = GRFGraphLabel<GraphType>::readGraph(graphPath.c_str());
	cout << "big graph ok" << endl;

	sg::SubgraphGenerator<GraphType> subgraphG(*graph, sNeed);

	subgraphG.run();
	cout << "subgraph generation is finished" << endl;
	const auto midGraph = subgraphG.getMid();
	writeMapping(midGraphPath, midGraph);
	auto queryGraph = subgraphG.getSmallGraph();
	graph->graphBuildFinish();
	queryGraph.graphBuildFinish();
	writeToFile(smallGraphPath, queryGraph);
	writeToFile(bigGraphPath, *graph);
	delete graph;
	delete randomer;

}