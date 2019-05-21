#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
#include<unordered_set>
#include<unordered_map>

using namespace std;


template<typename NodeType, typename EdgeType>
class  State {
public:
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef Graph<NodeType, EdgeType> GraphType;
	typedef const NodeType*   NodeCPointer;
	typedef KVPair<NodeCPointer, NodeCPointer> MapPair;
	typedef unordered_set<MapPair> MapType;
public:
	State() = default;
	~State() = default;

	virtual vector<MapPair> calCandidatePairs() {
		return vector<MapPair>
			();
	}
	virtual bool checkCanditatePairIsAddable(const MapPair cp) { return false; }
	virtual void addCanditatePairToMapping(const MapPair cp) { return; }
	virtual void deleteCanditatePairToMapping(const MapPair cp) {}
	virtual bool isCoverQueryGraph()const { return false; }
	virtual MapType getMap() const {
		if (isCoverQueryGraph() == false) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return MapType();
	}
};



template<typename NodeType, typename EdgeType>
class StateVF2 : public State<NodeType, EdgeType> {
public:
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef Graph<NodeType, EdgeType> GraphType;
	typedef const NodeType*   NodeCPointer;
	typedef KVPair<NodeCPointer, NodeCPointer> MapPair;
	typedef unordered_set<MapPair> MapType;
private:
	//private member;

	MapType mapping; //from query to target
	const GraphType &targetGraph, &queryGraph;
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
		if (NodeType::isSameTypeNode(*queryNode, *targetNode) && targetNode >= queryNode)return true;
		return false;
	}

	//sourceRule targetRule now is only usable to subgraph isomorphism but not induce s~~ i~;

	bool sourceRule(const MapPair &cp)
	{
		const auto &queryNode = cp.getKey();
		const auto &targetNode = cp.getValue();

		for (const auto &tempEdge : queryNode->getOutEdges()) {
			const auto &tempNodeID = tempEdge.getTargetNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) == queryGraphUnmap.end()) {
				if (targetNode->existSameTypeEdgeToNode(*tempNode, tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool targetRule(const MapPair &cp)
	{
		const auto &queryNode = cp.getKey();
		const auto &targetNode = cp.getValue();

		for (const auto &tempEdge : queryNode->getInEdges()) {
			const auto &tempNodeID = tempEdge.getSourceNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) == queryGraphUnmap.end()) {
	//			if (targetNode->existSameTypeEdgeFromNode(queryGraphUnmap.find(tempNode)getSecond(), tempEdge) == false) return false;
				if (targetNode->existSameTypeEdgeFromNode(*tempNode, tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool inRule(const MapPair &cp)
	{
		const auto &queryNode = cp.getKey();
		const auto &targetNode = cp.getValue();
		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0;

		for (const auto &tempEdge : queryNode->getInEdges())
		{
			const auto &tempNodeID = tempEdge.getSourceNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) != queryGraphUnmap.end())
			{
				if (queryMappingIn.find(tempNode) != queryMappingIn.end()) ++queryInCount;
				else ++queryNotTCount;
			}
		}
		for (const auto &tempEdge : targetNode->getInEdges())
		{
			const auto &tempNodeID = tempEdge.getSourceNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingIn.find(tempNode) != targetMappingIn.end()) ++targetInCount;
				else ++targetNotTCount;
			}
		}
		if (queryInCount > targetInCount || queryNotTCount > targetNotTCount) return false;
		else return true;
	}

	bool outRule(const MapPair &cp)
	{
		const auto &queryNode = cp.getKey();
		const auto &targetNode = cp.getValue();
		size_t queryOutCount = 0, targetOutCount = 0, queryNotTCount = 0, targetNotTCount = 0;

		for (const auto &tempEdge : queryNode->getOutEdges())
		{
			const auto &tempNodeID = tempEdge.getTargetNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (queryGraphUnmap.find(tempNode) != queryGraphUnmap.end())
			{
				if (queryMappingOut.find(tempNode) != queryMappingOut.end()) ++queryOutCount;
				else ++queryNotTCount;
			}
		}
		for (const auto &tempEdge : targetNode->getInEdges())
		{
			const auto &tempNodeID = tempEdge.getTargetNodeID();
			const auto &tempNode = queryGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingOut.find(tempNode) != targetMappingOut.end()) ++targetOutCount;
				else ++targetNotTCount;
			}
		}
		if (queryOutCount > targetOutCount || queryNotTCount > targetNotTCount) return false;
		else return true;
	}
	bool notInOrOutRule(const MapPair &cp)
	{

	}
public:
	StateVF2(const GraphType& _t, const GraphType& _q) :State<NodeType, EdgeType>(), targetGraph(_t), queryGraph(_q) {
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
	virtual vector<MapPair> calCandidatePairs()
	{
		{
			vector<MapPair> answer;
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
					answer.push_back(MapPair(queryNodeToMatch, temptargetNode));
				}
			}
			return answer;
		}
	}
	virtual bool checkCanditatePairIsAddable(const MapPair &cp)
	{
		return (sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp));

	}
	virtual void addCanditatePairToMapping(const MapPair &cp)
	{
		mapping.insert(cp);

		const auto targetNode = cp.getValue();
		const auto queryNode = cp.getKey();

		targetGraphUnmap.erase(targetNode);
		targetMappingIn.erase(targetNode);
		targetMappingOut.erase(targetNode);
		queryGraphUnmap.erase(queryNode);
		queryMappingIn.erase(queryNode);
		queryMappingOut.erase(queryNode);

		searchDepth++;

		for (const auto &tempEdge : targetNode->getInEdges()) {
			NodeType* tempNodePointer = targetGraph.getNodePointer(tempEdge.getSourceNodeID());
			// was not be mapped
			if (targetGraphUnmap.find(tempNodePointer) != targetGraphUnmap.end()) {
				targetMappingIn.insert(tempNodePointer);
				if (targetMappingInDepth[tempNodePointer] == 0) targetMappingInDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : targetNode->getOutEdges()) {
			auto tempNodePointer = targetGraph.getNodePointer(tempEdge.getTargetNodeID());
			if (targetGraphUnmap.find(tempNodePointer) != targetGraphUnmap.end()) {
				targetMappingOut.insert(tempNodePointer);
				if (targetMappingOutDepth[tempNodePointer] == 0) targetMappingOutDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNode->getOutEdges()) {
			auto tempNodePointer = queryGraph.getNodePointer(tempEdge.getTargetNodeID());
			if (queryGraphUnmap.find(tempNodePointer) != queryGraphUnmap.end()) {
				queryMappingOut.insert(tempNodePointer);
				if (queryMappingOutDepth[tempNodePointer] == 0) queryMappingOutDepth[tempNodePointer] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNode->getInEdges()) {
			auto tempNodePointer = queryGraph.getNodePointer(tempEdge.getSourceNodeID());
			if (queryGraphUnmap.find(tempNodePointer) != queryGraphUnmap.end()) {
				queryMappingIn.insert(tempNodePointer);
				if (queryMappingInDepth[tempNodePointer] == 0) queryMappingInDepth[tempNodePointer] = searchDepth;
			}
		}

		return;
	}
	virtual void deleteCanditatePairToMapping(const MapPair &cp)
	{
		try {
			//		if (mapping.top() != cp) throw("State.hpp deleteCanditatePairToMapping()");
			mapping.erase(cp);
			auto queryNode = cp.getKey();
			auto targetNode = cp.getValue();

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
	virtual bool isCoverQueryGraph()const {
		if (queryGraph.graphSize() == mapping.size())return true;
		return false;
	};
	virtual MapType getMap() const {
		if (isCoverQueryGraph() == false) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
};




