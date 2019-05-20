#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
#include<unordered_set>
#include<unordered_map>
using namespace std;
typedef const Node *  NodeCPointer; 
typedef FSPair<NodeCPointer, NodeCPointer> MappingPair;
//typedef KVPair<const Node const *, int> 
class  State {
	
public:
	State() = default;
	~State() = default;
	virtual vector<MappingPair> calCandidatePairs() {
		return vector<MappingPair>
			();
	}
	virtual bool checkCanditatePairIsAddable(MappingPair cp) { return false; }
	virtual void addCanditatePairToMapping(MappingPair cp) { return; }
	virtual void deleteCanditatePairToMapping(MappingPair cp) {}
	virtual bool isCoverQueryGraph() { return false; }
};

class StateVF2 : public State {
	
private:
	//private member;
	
	vector<MappingPair> mapping; //from query to target
	const Graph &targetGraph, &queryGraph;
//	unordered_set<KVPair<NodeCPointer, int>> targetMappingIn, targetMappingOut, queryMappingIn, queryMappingOut;
	unordered_set<NodeCPointer> targetGraphUnmap, targetMappingIn, targetMappingOut,
								 queryGraphUnmap,  queryMappingIn,  queryMappingOut;
	unordered_map<NodeCPointer, int> targetMappingInDepth, targetMappingOutDepth, queryMappingInDepth, queryMappingOutDepth;
	uint32_t searchDepth;


private:

	//private function
	size_t calUOS_reserveSize(size_t need) {
		size_t i = 16;
		while (i < need) i = i << 1;
		if (i * 0.9 > need) return i;
		else return i << 1;
	}
//	NodeCPointer selectNodeToCalCanditate(const unordered_set<KVPair<NodeCPointer, int>> &nodeSet);
	NodeCPointer selectNodeToCalCanditate(const unordered_set<NodeCPointer> &nodeSet);
	bool twoNodeMayMatch(NodeCPointer queryNode, NodeCPointer targetNode);
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
	virtual vector<MappingPair> calCandidatePairs();
	virtual bool checkCanditatePairIsAddable(MappingPair cp);
	virtual void addCanditatePairToMapping(MappingPair cp);
	virtual void deleteCanditatePairToMapping(MappingPair cp);
	virtual bool isCoverQueryGraph() {
		if (queryGraph.graphSize() == mapping.size())return true;
		return false;
	};
};
inline vector<MappingPair> StateVF2::calCandidatePairs()
{
	vector<MappingPair> answer;
	const unordered_set<NodeCPointer> * queryNodeToMatchSetPointer, *targetNodeToMatchSetPointer;
	if (queryMappingOut.size() > 0) {
		queryNodeToMatchSetPointer = &queryMappingOut;
		targetNodeToMatchSetPointer = &targetMappingOut;
	}
	else if (queryMappingIn.size() > 0) {
		queryNodeToMatchSetPointer = &queryMappingIn;
		targetNodeToMatchSetPointer = &targetMappingIn;
	}
	else {
		queryNodeToMatchSetPointer = &queryGraphUnmap;
		targetNodeToMatchSetPointer = &targetGraphUnmap;
	}
	const auto &queryNodeToMatchSet = *queryNodeToMatchSetPointer;
	const auto &targetNodeToMatchSet = *targetNodeToMatchSetPointer;
	auto queryNodeToMatch = selectNodeToCalCanditate(queryNodeToMatchSet);
	for (const auto &temptargetNode : targetNodeToMatchSet) {
		if (twoNodeMayMatch(queryNodeToMatch, temptargetNode)) {
			answer.push_back(MappingPair(queryNodeToMatch, temptargetNode));
		}
	}
	return answer;
}
void StateVF2::addCanditatePairToMapping(MappingPair cp) {
	return;
}
inline void StateVF2::deleteCanditatePairToMapping(MappingPair cp)
{
	return;
}
bool StateVF2::checkCanditatePairIsAddable(MappingPair cp) {
	return false;
}

inline NodeCPointer StateVF2::selectNodeToCalCanditate(const unordered_set<NodeCPointer>& nodeSet)
{
	return NodeCPointer();
}
inline bool StateVF2::twoNodeMayMatch(NodeCPointer queryNode, NodeCPointer targetNode)
{
	if (Node::isSameTypeNode(*queryNode, *targetNode) && targetNode >= queryNode)return true;
	return false;
}