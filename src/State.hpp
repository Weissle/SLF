#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
#include<unordered_set>
using namespace std;
class  State {

public:
	State() = default;
	~State() = default;
	virtual vector<MappingPair<Node, Node>> calCandidatePairs() {
		return vector<MappingPair<Node, Node>>
			();
	}
	virtual bool checkCanditatePairIsAddable(MappingPair<Node, Node> cp) { return false; }
	virtual void addCanditatePairToMapping(Pair<Node, Node> cp) { return; }
	virtual void deleteCanditatePairToMapping(Pair<Node, Node> cp) {}
	virtual bool isCoverQueryGraph() { return false; }
};

class StateVF2 : public State {
	vector<MappingPair<Node, Node>> mapping;
	const Graph &targetGraph, &queryGraph;
	unordered_set<Pair<Node*, int>> targetMappingIn, targetMappingOut, queryMappingIn, queryMappingOut;
	unordered_set<const Node*> targetGraphUnmap, queryGraphUnmap;
	uint32_t searchDepth;

	size_t calUOS_reserveSize(size_t need) {
		size_t i = 16;
		while (i < need) i = i << 1;
		if (i * 0.9 > need) return i;
		else return i << 1;
	}
public:
	StateVF2(const Graph& _t, const Graph& _q) :targetGraph(_t), queryGraph(_q) {
		mapping.resize(0);
		mapping.reserve(queryGraph.graphSize() + 5);
		targetMappingIn.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		targetMappingOut.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		targetGraphUnmap.reserve(calUOS_reserveSize(targetGraph.graphSize()));

		queryMappingIn.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		queryMappingOut.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		queryGraphUnmap.reserve(calUOS_reserveSize(queryGraph.graphSize()));

		searchDepth = 0;

		for (auto &tempNodePointer : targetGraph.getAllNodes()) targetGraphUnmap.insert(&tempNodePointer);
		for (auto &tempNodePointer : queryGraph.getAllNodes()) queryGraphUnmap.insert(&tempNodePointer);
	};
	StateVF2() = default;
	~StateVF2() = default;
	virtual vector<MappingPair<Node, Node>> calCandidatePairs();
	virtual bool checkCanditatePairIsAddable(MappingPair<Node, Node> cp);
	virtual void addCanditatePairToMapping(MappingPair<Node, Node> cp);
	virtual void deleteCanditatePairToMapping(MappingPair<Node, Node> cp);
	virtual bool isCoverQueryGraph() {
		if (queryGraph.graphSize() == mapping.size())return true;
		return false;
	};
};
inline vector<MappingPair<Node, Node>> StateVF2::calCandidatePairs()
{

	return vector<MappingPair<Node, Node>>();
}
void StateVF2::addCanditatePairToMapping(MappingPair<Node, Node> cp) {
	return;
}
inline void StateVF2::deleteCanditatePairToMapping(MappingPair<Node, Node> cp)
{
	return;
}
bool StateVF2::checkCanditatePairIsAddable(MappingPair<Node, Node> cp) {
	return false;
}