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
	typedef unordered_map<NodeCPointer, NodeCPointer> MapType;
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
	//	typedef unordered_set<MapPair> MapType;
	typedef unordered_map<NodeCPointer, NodeCPointer> MapType;
private:
	//private member;

	MapType mapping; //from query to target
	const GraphType &targetGraph, &queryGraph;
	unordered_set<NodeCPointer> targetGraphUnmap, targetMappingIn, targetMappingOut,
		queryGraphUnmap, queryMappingIn, queryMappingOut;
	unordered_map<NodeCPointer, int> targetMappingInDepth, targetMappingOutDepth, queryMappingInDepth, queryMappingOutDepth;
	uint32_t searchDepth;
	bool induceGraph = true;

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
	bool twoNodesMayMatch(NodeCPointer queryNodePointer, NodeCPointer targetNodePointer)
	{
		if (queryNodePointer->isSameType(*targetNodePointer) && *targetNodePointer >= *queryNodePointer)return true;
		return false;
	}

	//sourceRule targetRule now is only usable to subgraph isomorphism but not induce s~~ i~;
	bool sourceRuleInduceSubgraph(const MapPair &cp,const int need) {
		const auto &targetSourceNode = cp.getValue();
		int have = 0;
		for (const auto &it : targetSourceNode->getOutEdges()) {
			const auto targetTargetNodeID = it.getTargetNodeID();
			const auto targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			if (targetGraphUnmap.find(targetTargetNodePointer) == targetGraphUnmap.end()) {
				//in the mapping
				++have;
			}
		}
		return have == need;

	}
	bool sourceRule(const MapPair &cp)
	{
		const auto &querySourceNode = cp.getKey();
		const auto &targetSourceNode = cp.getValue();
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : querySourceNode->getOutEdges()) {
			//from queryNode to tempNode
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();

			const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			//this tempnode have been mapped
			auto temp1 = queryGraphUnmap.find(queryTargetNodePointer);
			auto temp2 = queryGraphUnmap.end();
			if (queryGraphUnmap.find(queryTargetNodePointer) == queryGraphUnmap.end()) {
				//		const auto &targetTargetNodePointer = mapping.find(queryTargetNodePointer).getValue();
				const auto &targetTargetNodePointer = mapping[queryTargetNodePointer];
				if (targetSourceNode->existSameTypeEdgeToNode(*targetTargetNodePointer, tempEdge) == false) return false;
				++nodeToMappingNum;
			}
		}
		if (induceGraph) return sourceRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}
	bool targetRuleInduceSubgraph(const MapPair &cp, const int need) {
		const auto &targetSourceNode = cp.getValue();
		int have = 0;
		for (const auto &it : targetSourceNode->getInEdges()) {
			const auto targetSourceNodeID = it.getSourceNodeID();
			const auto targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (targetGraphUnmap.find(targetSourceNodePointer) == targetGraphUnmap.end()) {
				//in the mapping
				++have;
			}
		}
		return have == need;

	}
	bool targetRule(const MapPair &cp)
	{
		const auto &queryTargetNode = cp.getKey();
		const auto &targetTargetNode = cp.getValue();
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : queryTargetNode->getInEdges()) {
			const auto &querySourceNodeID = tempEdge.getSourceNodeID();
			const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
			if (queryGraphUnmap.find(querySourceNodePointer) == queryGraphUnmap.end()) {
				nodeToMappingNum++;
				const auto &targetSourceNodePointer = mapping[querySourceNodePointer];
				if (targetTargetNode->existSameTypeEdgeFromNode(*targetSourceNodePointer, tempEdge) == false) return false;
			}
		}
		if (induceGraph) return targetRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
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
			const auto &tempNode = targetGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingIn.find(tempNode) != targetMappingIn.end()) ++targetInCount;
				else ++targetNotTCount;
			}
		}
		if (queryInCount > targetInCount /*|| queryNotTCount > targetNotTCount*/) return false;
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
		for (const auto &tempEdge : targetNode->getOutEdges())
		{
			const auto &tempNodeID = tempEdge.getTargetNodeID();
			const auto &tempNode = targetGraph.getNodePointer(tempNodeID);
			if (targetGraphUnmap.find(tempNode) != targetGraphUnmap.end())
			{
				if (targetMappingOut.find(tempNode) != targetMappingOut.end()) ++targetOutCount;
				else ++targetNotTCount;
			}
		}
		if (queryOutCount > targetOutCount/* || queryNotTCount > targetNotTCount*/) return false;
		else return true;
	}
	bool notInOrOutRule(const MapPair &cp)
	{

	}
public:
	StateVF2(const GraphType& _t, const GraphType& _q, bool _induceGraph) :State<NodeType, EdgeType>(), targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {
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

		for (auto &tempNode : targetGraph.getAllNodes()) targetGraphUnmap.insert(&tempNode);
		for (auto &tempNode : queryGraph.getAllNodes()) {
			auto temp = &tempNode;
			queryGraphUnmap.insert(&tempNode);
		}
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
		bool answer = (sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp));
		return answer;

	}
	virtual void addCanditatePairToMapping(const MapPair &cp)
	{
		//		mapping.insert(cp);
		mapping[cp.getKey()] = cp.getValue();
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
		auto deletePairFunction = [](unordered_map<NodeCPointer, int> &m, vector<NodeCPointer> &v) {
			for (const auto &it : v) {
				m.erase(it);
			}
			v.empty();
		};
		try {
			//		if (mapping.top() != cp) throw("State.hpp deleteCanditatePairToMapping()");
			mapping.erase(cp.getKey());
			auto queryNode = cp.getKey();
			auto targetNode = cp.getValue();

			targetGraphUnmap.insert(targetNode);
			queryGraphUnmap.insert(queryNode);

			vector<NodeCPointer> deleteList;
			for (const auto& tempPair : queryMappingInDepth) {
				if (tempPair.second == searchDepth) {
					queryMappingIn.erase(tempPair.first);
					//		queryMappingInDepth.erase(tempPair.first);
					deleteList.push_back(tempPair.first);
				}
			}
			deletePairFunction(queryMappingInDepth, deleteList);

			for (const auto& tempPair : queryMappingOutDepth) {
				if (tempPair.second == searchDepth) {
					queryMappingOut.erase(tempPair.first);
					//			queryMappingOutDepth.erase(tempPair.first);
					deleteList.push_back(tempPair.first);
				}
			}
			deletePairFunction(queryMappingOutDepth, deleteList);
			for (const auto& tempPair : targetMappingInDepth) {
				if (tempPair.second == searchDepth) {
					targetMappingIn.erase(tempPair.first);
					//			targetMappingInDepth.erase(tempPair.first);
					deleteList.push_back(tempPair.first);
				}
			}
			deletePairFunction(targetMappingInDepth, deleteList);

			for (const auto& tempPair : targetMappingOutDepth) {
				if (tempPair.second == searchDepth) {
					targetMappingOut.erase(tempPair.first);
					//			targetMappingOutDepth.erase(tempPair.first);
					deleteList.push_back(tempPair.first);
				}
			}
			deletePairFunction(targetMappingOutDepth, deleteList);
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




