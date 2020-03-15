#include"tools/GraphReader.hpp"
#include"tools/SubgraphGenerator.hpp"
#include<string>
#include<tools/RandomGenerator.hpp>
#include<tools/IndexTurner.hpp>
#include"tools/argh.h"
#include<thread>
using namespace std;
using namespace wg;
template<class GraphType>
void writeToFile(const string& fileName, const GraphType& graph) {
	ofstream f;
	f.flush();
	f.open(fileName.c_str(), ios::out);
	if (f.is_open() == false) {
		cout << fileName << " : open fail" << endl;
		return;
	}
	f << graph.size() << endl;
	for (auto& node : graph.nodes()) f << node.id() << " " << node.label() + 1 << endl;
	for (auto& node : graph.nodes()) {
		f << node.outEdgesNum() << endl;
		for (auto edge : node.outEdges()) {
			f << node.id() << " " << edge.target() << endl;
		}
	}
	f.close();
}

template<typename NodeIDType>
void writeMapping(const string& fileName, const vector<NodeIDType>& nodes) {
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
int main(int argc, char* argv[]) {

	argh::parser cmdl({ "-graph","-small-node" ,"-mg","-sg","big-node","-edge-avg","-labels","-bg","-randomModel","-variance","-labelTypes" });
	cmdl.parse(argc, argv);
	string graphPath, midGraphPath, smallGraphPath, bigGraphPath, randomModel = "normal";
	int sNeed = 0, bNeed = 0, labelTypes = 5;
	double variance = 1;
	double edgeavg = 1;
	cmdl({ "-graph" }) >> graphPath;
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
	assert((graphPath.empty() ^ bNeed == 0));
	typedef size_t NodeIDType;
	typedef Edge<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;

	auto time1 = clock();

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
		int Max = max(0, (int)((double)(2 * m + t) / 2));
		int Min = max(0, (int)(Max - t));
		randomer = new rg::MeanlyRandomGenerator(Min, Max);

	}
	GraphType* graph = nullptr;
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
		graph = new GraphType(nodes);
		vector<NodeIDType> in(bNeed);
		rg::NoRepeatIntRandomGenerator fnp(bNeed - 1);
		int inP = 1, noP = bNeed - 1;
		in[0] = bNeed - 1;
		LOOP(i, 0, bNeed - 1) {
			auto one = rand() % inP;
			auto to = fnp.getOne();
			if (rand() % 2) graph->addEdge(in[one], to);
			else graph->addEdge(to, in[one]);
			in[inP++] = to;
		}
		LOOP(i, 0, bNeed) {
			int edgeNum = min(max(0, (int)randomer->getOne()), bNeed - 2);
			rg::NoRepeatIntRandomGenerator irg(bNeed);

			auto outEdges = graph->node(i).outEdges();
			unordered_set<NodeIDType> exclude;
			exclude.reserve(bNeed);
			for (const auto& edge : outEdges) exclude.insert(edge.target());
			LOOP(j, 0, edgeNum) {
				NodeIDType to = irg.getOne();
				if (i == to || IN_SET(exclude, to)) {
					--j;
					continue;
				}
				if (to == -1) break;
				graph->addEdge(i, to);
			}
		}
	}
	else {
		IndexTurner<size_t> turner;
		graph = GRFGraphLabel<GraphType,size_t>::readGraph(graphPath.c_str(),turner);
	}
	cout << "big graph ok" << endl;

	sg::SubgraphGenerator<GraphType> subgraphG(*graph, sNeed);

	subgraphG.run();
	cout << "subgraph generation is finished" << endl;
	const auto midGraph = subgraphG.getMid();
	writeMapping(midGraphPath, midGraph);
	thread t1(writeMapping<NodeIDType>, ref(midGraphPath), ref(midGraph));
	auto queryGraph = subgraphG.getSmallGraph();

	auto fun = [](GraphType& g, const string& path) {
		g.graphBuildFinish();
		writeToFile(path, g);
	};
	thread t2(fun, ref(*graph), ref(bigGraphPath));// graph->graphBuildFinish();
	thread t3(fun, ref(queryGraph), ref(smallGraphPath));// queryGraph.graphBuildFinish();
	t1.join();
	t2.join();
	t3.join();

	/*
		writeToFile(smallGraphPath, queryGraph);
		writeToFile(bigGraphPath, *graph);

		graph->graphBuildFinish();
		queryGraph.graphBuildFinish();
		writeToFile(smallGraphPath, queryGraph);
		writeToFile(bigGraphPath, *graph);*/
	delete graph;
	delete randomer;

	PRINT_TIME_COST_S("time cost : ", clock() - time1);
	return 0;
}