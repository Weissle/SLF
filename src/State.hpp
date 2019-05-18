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
	virtual vector<Pair<Node, Node>> calCandidatePairs() {
		return vector<Pair<Node, Node>>
			();
	}
	virtual bool addCanditatePairToMapping(Pair<Node, Node> cp) { return false; }
	virtual void deleteCanditatePairToMapping(Pair<Node, Node> cp) {}
	virtual bool isCoverQueryGraph() { return false; }
};

class StateVF2 : public State {
	vector<Pair<Node, Node>> mapping;
	const Graph &targetGraph, &queryGraph;


public:
	StateVF2(const Graph &_t, const Graph &_q) :targetGraph(_t), queryGraph(_q);
	StateVF2() = default;
	~StateVF2() = default;
	virtual vector<Pair<Node, Node>> calCandidatePairs();
	virtual bool addCanditatePairToMapping(Pair<Node, Node> cp);
	virtual void deleteCanditatePairToMapping(Pair<Node, Node> cp);
	virtual bool isCoverQueryGraph() { 
		if (queryGraph.graphSize() == mapping.size())return true; 
	};
};