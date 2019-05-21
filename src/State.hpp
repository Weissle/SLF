#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
#include<unordered_set>
#include<unordered_map>

using namespace std;


template<typename NodeIDType, typename EdgeLabelType, typename NodeLabelType>
class  State {
	typedef Edge<NodeIDType, EdgeLabelType> EdgeType;
	typedef const Node<NodeIDType, EdgeType, NodeLabelType>*   NodeCPointer;
	typedef FSPair<NodeCPointer, NodeCPointer> MappingPair;
public:
	State() = default;
	~State() = default;

	virtual vector<MappingPair> calCandidatePairs() {
		return vector<MappingPair>
			();
	}
	virtual bool checkCanditatePairIsAddable(const MappingPair cp) { return false; }
	virtual void addCanditatePairToMapping(const MappingPair cp) { return; }
	virtual void deleteCanditatePairToMapping(const MappingPair cp) {}
	virtual bool isCoverQueryGraph() { return false; }
};



template<typename NodeIDType, typename EdgeLabelType, typename NodeLabelType>
class StateVF2 : public State<NodeIDType, EdgeLabelType, NodeLabelType> {
	typedef Edge<NodeIDType, EdgeLabelType> EdgeType;
	typedef const Node<NodeIDType, EdgeType, NodeLabelType>*   NodeCPointer;
	typedef KVPair<NodeCPointer, NodeCPointer> MappingPair;
private:
	//private member;

	unordered_set<MappingPair> mapping; //from query to target
	const Graph<NodeIDType, EdgeLabelType, NodeLabelType> &targetGraph, &queryGraph;
	unordered_set<NodeCPointer> targetGraphUnmap, targetMappingIn, targetMappingOut,
		queryGraphUnmap, queryMappingIn, queryMappingOut;
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

	NodeCPointer selectNodeToCalCanditates(const unordered_set<NodeCPointer> &nodeSet)
	{
		return *nodeSet.begin();
	}
	//same label and target node edges' number should more than query node's.
	bool twoNodesMayMatch(NodeCPointer queryNode, NodeCPointer targetNode)
	{
		if (Node::isSameTypeNode(*queryNode, *targetNode) && targetNode >= queryNode)return true;
		return false;
	}

	//sourceRule targetRule now is only usable to subgraph isomorphism but not induce s~~ i~;

	bool sourceRule(const MappingPair &cp)
	{
		const auto &queryNode = cp.first();
		const auto &targetNode = cp.second();

		for (const auto &tempEdge : queryNode.getOutEdges()) {
			const auto &tempNodeID = tempEdge.getTargetNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) == queryGraphUnmap.end()) {
				if (targetNode.existSameTypeEdgeToNode(*queryGraphUnmap.find(tempNode).getSecond(), tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool targetRule(const MappingPair &cp)
	{
		const auto &queryNode = cp.first();
		const auto &targetNode = cp.second();

		for (const auto &tempEdge : queryNode.getInEdges()) {
			const auto &tempNodeID = tempEdge.getSourceNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) == queryGraphUnmap.end()) {
				if (targetNode.existSameTypeEdgeFromNode(*queryGraphUnmap.find(tempNode).getSecond(), tempEdge) == false) return false;
			}
		}
	}
	bool inRule(const MappingPair &cp)
	{
		const auto &queryNode = cp.first();
		const auto &targetNode = cp.second();
		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTInCount = 0;

		for (const auto &tempEdge : queryNode.getInEdges())
		{
			const auto &tempNodeID = tempEdge.getSourceNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) != queryGraphUnmap.end())
			{
				if (queryMappingIn.find(tempNode) != queryMappingIn.end()) ++queryInCount;
				else ++queryNotTCount;
			}
		}
		for (const auto &tempEdge : targetNode.getInEdges())
		{
			const auto &tempNodeID = tempEdge.getSourceNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingIn.find(tempNode) != targetMappingIn.end()) ++targetInCount;
				else ++targetNotTCount;
			}
		}
		if (queryInCount > targetInCount || queryNotTCount > targetNotInCount) return false;
	}

	bool outRule(const MappingPair &cp)
	{
		const auto &queryNode = cp.first();
		const auto &targetNode = cp.second();
		size_t queryOutCount = 0, targetOutCount = 0, queryNotTCount = 0, targetNotTInCount = 0;

		for (const auto &tempEdge : queryNode.getOutEdges())
		{
			const auto &tempNodeID = tempEdge.getTargetNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) != queryGraphUnmap.end())
			{
				if (queryMappingOut.find(tempNode) != queryMappingOut.end()) ++queryOutCount;
				else ++queryNotTCount;
			}
		}
		for (const auto &tempEdge : targetNode.getInEdges())
		{
			const auto &tempNodeID = tempEdge.getTargetNode();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingOut.find(tempNode) != targetMappingOut.end()) ++targetOutCount;
				else ++targetNotTCount;
			}
		}
		if (queryOutCount > targetOutCount || queryNotTCount > targetNotInCount) return false;
	}
	bool notInOrOutRule(const MappingPair &cp)
	{

	}
public:
	StateVF2(const Graph& _t, const Graph& _q) :targetGraph(_t), queryGraph(_q) {
		//	mapping.
		//	mapping.resize(0);
		mapping.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		targetMappingIn.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		targetMappingOut.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		targetGraphUnmap.reserve(calUOS_reserveSize(targetGraph.graphSize()));

		queryMappingIn.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		queryMappingOut.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		queryGraphUnmap.reserve(calUOS_reserveSize(queryGraph.graphSize()));

		targetMappingInDepth.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		targetMappingOutDepth.reserve(calUOS_reserveSize(targetGraph.graphSize()));
		queryMappingInDepth.reserve(calUOS_reserveSize(queryGraph.graphSize()));
		queryMappingOutDepth.reserve(calUOS_reserveSize(queryGraph.graphSize()));

		searchDepth = 0;

		for (auto &tempNodePointer : targetGraph.getAllNodes()) targetGraphUnmap.insert(&tempNodePointer);
		for (auto &tempNodePointer : queryGraph.getAllNodes()) queryGraphUnmap.insert(&tempNodePointer);
	};
	StateVF2() = default;
	~StateVF2() = default;

public:
	//public function
	virtual vector<MappingPair> calCandidatePairs()
	{
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
			auto queryNodeToMatch = selectNodeToCalCanditates(queryNodeToMatchSet);
			for (const auto &temptargetNode : targetNodeToMatchSet) {
				if (twoNodesMayMatch(queryNodeToMatch, temptargetNode)) {
					answer.push_back(MappingPair(queryNodeToMatch, temptargetNode));
				}
			}
			return answer;
		}
	}
	virtual bool checkCanditatePairIsAddable(const MappingPair &cp)
	{
		return (sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp));

	}
	virtual void addCanditatePairToMapping(const MappingPair &cp)
	{
		mapping.insert(cp);

		const auto targetNode = cp.getSecond();
		const auto queryNode = cp.getFirst();

		targetGraphUnmap.erase(targetNode);
		targetMappingIn.erase(targetNode);
		targetMappingOut.erase(targetNode);
		queryGraphUnmap.erase(queryNode);
		queryMappingIn.erase(queryNode);
		queryMappingOut.erase(queryNode);

		searchDepth++;

		for (const auto &tempEdge : targetNode->getInEdges()) {

			auto tempNodePointer = targetGraph.getNodePointer(tempEdge.getSourceNode());
			// was not be mapped
			if (targetGraphUnmap.find(tempNodePointer) != targetGraphUnmap.end()) {
				targetMappingIn.insert(tempNodePointer);
				if (targetMappingInDepth[tempNodePointer] == 0) targetMappingInDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : targetNode->getOutEdges()) {
			auto tempNodePointer = targetGraph.getNodePointer(tempEdge.getTargetNode());
			if (targetGraphUnmap.find(tempNodePointer) != targetGraphUnmap.end()) {
				targetMappingOut.insert(tempNodePointer);
				if (targetMappingOutDepth[tempNodePointer] == 0) targetMappingOutDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNode->getOutEdges()) {
			auto tempNodePointer = queryGraph.getNodePointer(tempEdge.getTargetNode());
			if (queryGraphUnmap.find(tempNodePointer) != queryGraphUnmap.end()) {
				queryMappingOut.insert(tempNodePointer);
				if (queryMappingOutDepth[tempNodePointer] == 0) queryMappingOutDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNode->getInEdges()) {
			auto tempNodePointer = queryGraph.getNodePointer(tempEdge.getSourceNode());
			if (queryGraphUnmap.find(tempNodePointer) != queryGraphUnmap.end()) {
				queryMappingIn.insert(tempNodePointer);
				if (queryMappingInDepth[tempNodePointer] == 0) queryMappingInDepth[tempNodePointer] = searchDepth;
			}
		}

		return;
	}
	virtual void deleteCanditatePairToMapping(const MappingPair &cp)
	{
		try {
			//		if (mapping.top() != cp) throw("State.hpp deleteCanditatePairToMapping()");
			mapping.erase(cp);
			auto queryNode = cp.getFirst();
			auto targetNode = cp.getSecond();

			targetGraphUnmap.insert(targetNode);
			queryGraphUnmap.insert(queryNode);

			for (const auto& tempPair : queryMappingInDepth) {
				if (tempPair.second == searchDepth) {
					queryMappingIn.erase(tempPair.first);
					queryMappingInDepth.erase(tempPair.first);
				}
			}
			for (const auto& tempPair : queryMappingOutDepth) {
				if (tempPair.second == searchDepth) {
					queryMappingOut.erase(tempPair.first);
					queryMappingOutDepth.erase(tempPair.first);
				}
			}
			for (const auto& tempPair : targetMappingInDepth) {
				if (tempPair.second == searchDepth) {
					targetMappingIn.erase(tempPair.first);
					targetMappingInDepth.erase(tempPair.first);
				}
			}
			for (const auto& tempPair : targetMappingOutDepth) {
				if (tempPair.second == searchDepth) {
					targetMappingOut.erase(tempPair.first);
					targetMappingOutDepth.erase(tempPair.first);
				}
			}
			searchDepth--;
		}
		catch (char *err)
		{
			cout << err << endl;

		}
		return;

	}
	virtual bool isCoverQueryGraph() {
		if (queryGraph.graphSize() == mapping.size())return true;
		return false;
	};
};




