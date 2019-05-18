#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
using namespace std;
class  State {

public:
	State() = default;
	~State() = default;
	virtual vector<MappingPair<Node, Node>> calCandidatePairs() {
		return vector<MappingPair<Node, Node>>
			();
	}
	virtual bool addCanditatePairToMapping(Pair<Node, Node> cp) { return false; }
	virtual void deleteCanditatePairToMapping(Pair<Node, Node> cp) {}
	virtual bool isCoverQueryGraph() { return false; }
};

class StateVF2 : public State {
	vector<MappingPair<Node, Node>> mapping;
	const Graph &targetGraph, &queryGraph;


public:
	StateVF2(const Graph& _t, const Graph& _q) :targetGraph(_t), queryGraph(_q) {};
	StateVF2() = default;
	~StateVF2() = default;
	virtual vector<MappingPair<Node, Node>> calCandidatePairs();
	virtual bool addCanditatePairToMapping(MappingPair<Node, Node> cp);
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
bool StateVF2::addCanditatePairToMapping(MappingPair<Node, Node> cp) {
	return false;
}
inline void StateVF2::deleteCanditatePairToMapping(MappingPair<Node, Node> cp)
{
	return;
}