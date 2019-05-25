#pragma once
#include<stack>
#include<vector>
#include"Node.hpp"
#include"Pair.hpp"
#include"Graph.hpp"
#include<assert.h>
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
using namespace std;


template<typename GraphType>
class  State {
public:
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;


	typedef const NodeType*   NodeCPointer;
	typedef KVPair<NodeIDType, NodeIDType> MapPair;
	typedef unordered_map<NodeIDType, NodeIDType> MapType;
public:
	State() = default;
	~State() = default;

	virtual vector<MapPair> calCandidatePairs()const = 0;
	virtual bool checkCanditatePairIsAddable(const MapPair &cp) const = 0;
	virtual void addCanditatePairToMapping(const MapPair &cp) = 0;
	virtual void deleteCanditatePairToMapping(const MapPair &cp) = 0;
	virtual bool isCoverQueryGraph()const = 0;
	virtual MapType getMap() const = 0;

};

bool boolForDebug = false;

template<typename GraphType>
class StateVF2 : public State<GraphType> {
public:
	typedef typename GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef typename State<GraphType> StateBase;
	typedef const NodeType*   NodeCPointer;
	typedef StateBase::MapPair MapPair;
	typedef StateBase::MapType MapType;
	typedef unordered_set<NodeIDType> NodeSetType;

private:

	MapType mapping, mappingAux; //from query to target
	const GraphType &targetGraph, &queryGraph;
	NodeSetType targetGraphUnmap, targetMappingIn, targetMappingOut,
		queryGraphUnmap, queryMappingIn, queryMappingOut;
	size_t targetMappingInSize = 0, targetMappingOutSize = 0, queryMappingInSize = 0, queryMappingOutSize = 0;

	unordered_map<NodeIDType, size_t> targetMappingInDepth, targetMappingOutDepth, queryMappingInDepth, queryMappingOutDepth;
	size_t searchDepth;

	vector<FSPair<NodeIDType, size_t>> matchSequence;
	bool induceGraph = true;

	void seePairID(const MapPair &cp)const {
		cout << "Ready To Mapping" << "(" << cp.getKey() << "," << cp.getValue() << ")" << endl;
	}
private:
	void seeMappingContent()const {
		for (const auto it : mapping) {
			if (it.second) {
				const auto queryNodeID = it.first;
				const auto targetNodeID = it.second;
				cout << '(' << queryNodeID << "," << targetNodeID << ')' << endl;

			}
		}
	}

	bool setContainNodeID(const NodeSetType &s, const NodeIDType &nodeID) const {
		return (s.find(nodeID) != s.end());
	}
	bool setNotContainNodeID(const NodeSetType &s, const NodeIDType &nodeID)const {
		return (s.find(nodeID) == s.end());
	}
	NodeIDType selectNodeToCalCanditates(const NodeSetType &nodeSet)const
	{
		return *nodeSet.begin();
	}
	NodeIDType selectNodeToCalCanditates()const
	{
		/*	const NodeSetType * queryNodeToMatchSetPointer, *targetNodeToMatchSetPointer;
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
			return *queryNodeToMatchSet.begin();*/
		return matchSequence[searchDepth].getFirst();
		/*	vector<FSPair<NodeType, size_t>> queryNode;
			queryNode.reserve(queryGraphUnmap.size());
			for (const auto &id : queryGraphUnmap) {
				const auto tempQueryNode = queryGraph.getNode(id);
				FSPair<NodeType, size_t> tempPair(id, tempQueryNode.getInEdgesNum() + tempQueryNode.getOutEdgesNum());
			}*/


	}
	//same label and target node edges' number should more than query node's.
	bool twoNodesMayMatch(NodeIDType queryNodeID, NodeIDType targetNodeID)const
	{
		const auto &queryNode = queryGraph.getNode(queryNodeID);
		const auto &targetNode = targetGraph.getNode(targetNodeID);
		return ((queryNode.isSameType(targetNode)) && (queryNode <= targetNode));

	}

	bool sourceRuleInduceSubgraph(const MapPair &cp, const int need)const {
		/*		const auto &targetSourceNode = cp.getValue();
				int have = 0;
				for (const auto &it : targetSourceNode->getOutEdges()) {
					const auto targetTargetNodeID = it.getTargetNodeID();
					const auto targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
					if (targetGraphUnmap.find(targetTargetNodePointer) == targetGraphUnmap.end()) {
						//in the mapping
						++have;
						if (have > need)return false;
					}
				}
				return have == need;*/
		const auto &querySourceNodeID = cp.getKey();
		const auto &targetSourceNodeID = cp.getValue();

		const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);

		for (const auto &tempEdge : targetSourceNodePointer->getOutEdges()) {
			const auto &targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			if (setNotContainNodeID(targetGraphUnmap, targetTargetNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetTargetNodeID);
				assert((tempMappingPair != mappingAux.end()) && "mapping not exist the map about this node");
				const auto &queryTargetNodeID = tempMappingPair->second;
				const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
				assert(queryTargetNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (querySourceNodePointer->existSameTypeEdgeToNode(*queryTargetNodePointer, tempEdge) == false) return false;
			}
		}
		return true;

	}
	bool sourceRule(const MapPair &cp) const
	{
		const auto &querySourceNodeID = cp.getKey();
		const auto &targetSourceNodeID = cp.getValue();

		const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : querySourceNodePointer->getOutEdges()) {
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();
			const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			//this tempnode have been mapped
			if (setNotContainNodeID(queryGraphUnmap, queryTargetNodeID)) {
				const auto tempMappingPair = mapping.find(queryTargetNodeID);
				assert((tempMappingPair != mapping.end()) && "mapping not exist the map about this node");
				const auto &targetTargetNodeID = tempMappingPair->second;
				const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
				if (targetSourceNodePointer->existSameTypeEdgeToNode(*targetTargetNodePointer, tempEdge) == false) return false;
				++nodeToMappingNum;
			}
		}
		if (induceGraph) return sourceRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}
	bool targetRuleInduceSubgraph(const MapPair &cp, const int need)const {
		/*		const auto &targetSourceNode = cp.getValue();
				int have = 0;
				for (const auto &it : targetSourceNode->getInEdges()) {
					const auto targetSourceNodeID = it.getSourceNodeID();
					const auto targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
					if (targetGraphUnmap.find(targetSourceNodePointer) == targetGraphUnmap.end()) {
						//in the mapping
						++have;
						if (have > need)return false;
					}
				}
				return have == need;*/
		const auto &queryTargetNodeID = cp.getKey();
		const auto &targetTargetNodeID = cp.getValue();

		const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);


		for (const auto &tempEdge : targetTargetNodePointer->getInEdges()) {
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);

			if (setNotContainNodeID(targetGraphUnmap, targetSourceNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetSourceNodeID);

				assert((tempMappingPair != mappingAux.end()) && "mapping not exist the map about this node");
				const auto &querySourceNodeID = tempMappingPair->second;
				const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
				assert(querySourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (queryTargetNodePointer->existSameTypeEdgeFromNode(*querySourceNodePointer, tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool targetRule(const MapPair &cp)const
	{
		const auto &queryTargetNodeID = cp.getKey();
		const auto &targetTargetNodeID = cp.getValue();

		const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

		int nodeToMappingNum = 0;
		for (const auto &tempEdge : queryTargetNodePointer->getInEdges()) {
			const auto &querySourceNodeID = tempEdge.getSourceNodeID();
			const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);

			if (setNotContainNodeID(queryGraphUnmap, querySourceNodeID)) {
				nodeToMappingNum++;
				const auto tempMappingPair = mapping.find(querySourceNodeID);
				if (tempMappingPair == mapping.end()) {
					for (auto it : queryGraphUnmap) cout << it << " " << endl;
				}
				assert((tempMappingPair != mapping.end()) && "mapping not exist the map about this node");
				const auto & targetSourceNodeID = tempMappingPair->second;
				const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
				assert(targetSourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (targetTargetNodePointer->existSameTypeEdgeFromNode(*targetSourceNodePointer, tempEdge) == false) return false;
			}
		}
		if (induceGraph) return targetRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}

	bool inRule(const MapPair &cp)const
	{
		const auto &queryTargetNodeID = cp.getKey();
		const auto &targetTargetNodeID = cp.getValue();

		const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0;
		size_t qtemp = 0, ttemp = 0;
		for (const auto &tempEdge : queryTargetNodePointer->getInEdges())
		{
			const auto &querySourceNodeID = tempEdge.getSourceNodeID();
			const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
			if (setContainNodeID(queryGraphUnmap, querySourceNodeID))
			{
				if (setContainNodeID(queryMappingIn, querySourceNodeID)) ++queryInCount;
				else ++queryNotTCount;
			}
			else ++qtemp;

		}
		for (const auto &tempEdge : targetTargetNodePointer->getInEdges())
		{
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (setContainNodeID(targetGraphUnmap, targetSourceNodeID))
			{
				if (setContainNodeID(targetMappingIn, targetSourceNodeID)) ++targetInCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		if (queryInCount > targetInCount || queryNotTCount > targetNotTCount) return false;
		else return true;
	}
	bool outRule(const MapPair &cp)const
	{

		const auto &querySourceNodeID = cp.getKey();
		const auto &targetSourceNodeID = cp.getValue();

		const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);


		size_t queryOutCount = 0, targetOutCount = 0, queryNotTCount = 0, targetNotTCount = 0;
		size_t qtemp = 0, ttemp = 0;
		for (const auto &tempEdge : querySourceNodePointer->getOutEdges())
		{
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();
			const auto & queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);

			if (setContainNodeID(queryGraphUnmap, queryTargetNodeID))
			{
				if (setContainNodeID(queryMappingOut, queryTargetNodeID)) ++queryOutCount;
				else ++queryNotTCount;
			}
			else ++qtemp;
		}
		for (const auto &tempEdge : targetSourceNodePointer->getOutEdges())
		{
			const auto &targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto & targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

			if (setContainNodeID(targetGraphUnmap, targetTargetNodeID))
			{
				if (setContainNodeID(targetMappingOut, targetTargetNodeID)) ++targetOutCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		if (queryOutCount > targetOutCount || queryNotTCount > targetNotTCount) return false;
		else return true;
	}
	bool notInOrOutRule(const MapPair &cp)
	{

	}
public:
	StateVF2(const GraphType& _t, const GraphType& _q, bool _induceGraph) :targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {

		auto calASizeForHash = [](const size_t need) {
			size_t i = 16;
			while (i < need) i = i << 1;
			if (i * 0.8 > need) return i;
			else return i << 1;
		};
		const int queryHashSize = calASizeForHash(queryGraph.graphSize());
		const int targetHashSize = calASizeForHash(targetGraph.graphSize());
		mapping.reserve(queryHashSize);
		mappingAux.reserve(targetHashSize);
		targetMappingIn.reserve(targetHashSize);
		targetMappingOut.reserve(targetHashSize);
		targetGraphUnmap.reserve(targetHashSize);

		queryMappingIn.reserve(queryHashSize);
		queryMappingOut.reserve(queryHashSize);
		queryGraphUnmap.reserve(queryHashSize);

		targetMappingInDepth.reserve(targetHashSize);
		targetMappingOutDepth.reserve(targetHashSize);
		queryMappingInDepth.reserve(queryHashSize);
		queryMappingOutDepth.reserve(queryHashSize);

		searchDepth = 0;
		for (auto &tempNode : targetGraph.getAllNodes())
		{
			targetGraphUnmap.insert(getNodeID(tempNode));
		}
		for (auto &tempNode : queryGraph.getAllNodes())
		{
			queryGraphUnmap.insert(getNodeID(tempNode));
		}

		matchSequence.reserve(queryGraphUnmap.size());
		for (const auto &id : queryGraphUnmap) {
			const auto tempQueryNode = queryGraph.getNode(id);
			FSPair<NodeIDType, size_t> tempPair(id, tempQueryNode.getInEdgesNum() + tempQueryNode.getOutEdgesNum());
			matchSequence.push_back(tempPair);
		}
		const auto cmpFunction = [](const FSPair<NodeIDType, size_t> a, const FSPair<NodeIDType, size_t> b) {
			return (a.getSecond() > b.getSecond());
			//		return (a->getSecond() > b->getSecond());
		};
		sort(matchSequence.begin(), matchSequence.end(), cmpFunction);

	};
	StateVF2() = default;
	~StateVF2() = default;

public:
	//public function
	vector<MapPair> calCandidatePairs()const
	{
	/*	if (queryMappingIn.size() != queryMappingInSize || queryMappingOut.size() != queryMappingOutSize
			|| targetMappingIn.size() != targetMappingInSize || targetMappingOut.size() != targetMappingOutSize) {
			cout << "error on" << endl;
		}
		const bool numOfInOut = ((queryMappingIn.size() <= targetMappingIn.size())
			&& (queryMappingOut.size() <= targetMappingOut.size())
			&& (queryGraphUnmap.size() <= targetGraphUnmap.size()));*/
		const bool numOfInOut = (queryMappingInSize <= targetMappingInSize)
			&& (queryMappingOutSize <= targetMappingOutSize);
		if (numOfInOut == false)return vector<MapPair>();
		vector<MapPair> answer;
		const auto &queryNodeToMatchID = selectNodeToCalCanditates();
		const bool queryNodeInIn = setContainNodeID(queryMappingIn, queryNodeToMatchID);
		const bool queryNodeInOut = setContainNodeID(queryMappingOut, queryNodeToMatchID);
		const NodeSetType *tempNodeSetPointer;

		if (queryNodeInIn) tempNodeSetPointer = &targetMappingIn;
		else if (queryNodeInOut)tempNodeSetPointer = &targetMappingOut;
		else tempNodeSetPointer = &targetGraphUnmap;

		const auto &targetNodeToMatchSet = *tempNodeSetPointer;
//		answer.reserve(targetGraph.graphSize()-searchDepth)
		//		const auto &queryNodeToMatchID = selectNodeToCalCanditates(queryNodeToMatchSet);
		for (const auto &targetNodeToMatchID : targetNodeToMatchSet) {
			const bool targetNodeInIn = setContainNodeID(targetMappingIn, targetNodeToMatchID);
			const bool targetNodeInOut = setContainNodeID(targetMappingOut, targetNodeToMatchID);
			if (twoNodesMayMatch(queryNodeToMatchID, targetNodeToMatchID) && (targetNodeInIn == queryNodeInIn) && (targetNodeInOut == queryNodeInOut))
			{
				answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));
			}
		}
		return answer;

	}
	bool checkCanditatePairIsAddable(const MapPair &cp)const
	{
		//	seePairID(cp);
		//	seeMappingContent(cp);
			//		bool answer = (sourceRule(cp) && targetRule(cp));/* && inRule(cp) && outRule(cp));*/
			//		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp);
			//		bool answer = sourceRule(cp) && targetRule(cp) && outRule(cp);
		
		bool answer =sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp);
		return answer;

	}
	void addCanditatePairToMapping(const MapPair &cp)
	{

		mapping[cp.getKey()] = cp.getValue();
		mappingAux[cp.getValue()] = cp.getKey();
		const auto targetNodeID = cp.getValue();
		const auto queryNodeID = cp.getKey();

		targetGraphUnmap.erase(targetNodeID);
		queryGraphUnmap.erase(queryNodeID);

		const auto reduceSetSizeAfterErase = [](NodeSetType &set, const NodeIDType &nodeID, size_t &size) {
	//		if (setContainNodeID(set, nodeID)) {
			if(set.find(nodeID)!=set.end()){
				--size;
				set.erase(nodeID);
			}
		};
		reduceSetSizeAfterErase(targetMappingIn, targetNodeID, targetMappingInSize);
		reduceSetSizeAfterErase(targetMappingOut, targetNodeID, targetMappingOutSize);
		reduceSetSizeAfterErase(queryMappingIn, queryNodeID, queryMappingInSize);
		reduceSetSizeAfterErase(queryMappingOut, queryNodeID, queryMappingOutSize);
	/*	if (setContainNodeID(targetMappingIn, targetNodeID))
		{
			--targetMappingInSize;
			targetMappingIn.erase(targetNodeID);
		}
		if (setContainNodeID(targetMappingOut, targetNodeID)) 
		{
			--targetMappingOutSize;
			targetMappingOut.erase(targetNodeID);
		}
		if (setContainNodeID(queryMappingIn, queryNodeID)) {
			--queryMappingInSize;
			queryMappingIn.erase(queryNodeID);
		}
		if (setContainNodeID(queryMappingOut, queryNodeID)) 
		{
			--queryMappingOutSize;
			queryMappingOut.erase(queryNodeID);
		}*/

		const auto &targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto &queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		for (const auto &tempEdge : targetNodePointer->getInEdges()) {
			const auto &sourceNodeID = tempEdge.getSourceNodeID();
			// was not be mapped
			if (setContainNodeID(targetGraphUnmap, sourceNodeID)) {
				targetMappingIn.insert(sourceNodeID);
				if (targetMappingInDepth[sourceNodeID] == 0) {
					targetMappingInDepth[sourceNodeID] = searchDepth;
					++targetMappingInSize;
				}
			}
		}
		for (const auto &tempEdge : targetNodePointer->getOutEdges()) {
			const auto &targetNodeID = tempEdge.getTargetNodeID();
			if (setContainNodeID(targetGraphUnmap, targetNodeID)) {
				targetMappingOut.insert(targetNodeID);
				if (targetMappingOutDepth[targetNodeID] == 0) {
					targetMappingOutDepth[targetNodeID] = searchDepth;
					++targetMappingOutSize;
				}
			}
		}
		for (const auto &tempEdge : queryNodePointer->getOutEdges()) {
			const auto &targetNodeID = tempEdge.getTargetNodeID();

			if (setContainNodeID(queryGraphUnmap, targetNodeID)) {
				queryMappingOut.insert(targetNodeID);
				if (queryMappingOutDepth[targetNodeID] == 0) {
					queryMappingOutDepth[targetNodeID] = searchDepth;
					++queryMappingOutSize;
				}
			}
		}
		for (const auto &tempEdge : queryNodePointer->getInEdges()) {
			const auto &sourceNodeID = tempEdge.getSourceNodeID();
			if (setContainNodeID(queryGraphUnmap, sourceNodeID)) {
				queryMappingIn.insert(sourceNodeID);
				if (queryMappingInDepth[sourceNodeID] == 0) {
					queryMappingInDepth[sourceNodeID] = searchDepth;
					++queryMappingInSize;
				}
			}
		}

		return;
	}
	void deleteCanditatePairToMapping(const MapPair &cp)
	{

		mapping.erase(cp.getKey());
		mappingAux.erase(cp.getValue());
		const auto &queryNodeID = cp.getKey();
		const auto &targetNodeID = cp.getValue();

		targetGraphUnmap.insert(targetNodeID);
		queryGraphUnmap.insert(queryNodeID);

//		vector<NodeCPointer> deleteList;
/*
		const auto reduceSetSizeAfterErase = [](NodeSetType &set, const NodeIDType &nodeID, size_t &size) {
			if (setContainNodeID(set, nodeID)) {
				--size;
				set.erase(nodeID);
			}
		};*/
		for (const auto& tempPair : queryMappingInDepth) {
			if (tempPair.second == searchDepth) {
				queryMappingIn.erase(tempPair.first);
				--queryMappingInSize;
				queryMappingInDepth[tempPair.first] = 0;

			}
		}
		if (queryMappingInDepth[queryNodeID] < searchDepth && queryMappingInDepth[queryNodeID]) {
			++queryMappingInSize;
			queryMappingIn.insert(queryNodeID);
		}

		for (const auto& tempPair : queryMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				--queryMappingOutSize;
				queryMappingOut.erase(tempPair.first);
				queryMappingOutDepth[tempPair.first] = 0;

			}
		}
		if (queryMappingOutDepth[queryNodeID] < searchDepth && queryMappingOutDepth[queryNodeID])
		{
			++queryMappingOutSize;
			queryMappingOut.insert(queryNodeID);
		}

		for (const auto& tempPair : targetMappingInDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingInSize--;
				targetMappingIn.erase(tempPair.first);
				targetMappingInDepth[tempPair.first] = 0;

			}
		}

		if (targetMappingInDepth[targetNodeID] < searchDepth && targetMappingInDepth[targetNodeID]) {
			targetMappingInSize++;
			targetMappingIn.insert(targetNodeID);
		}
		for (const auto& tempPair : targetMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingOutSize--;
				targetMappingOut.erase(tempPair.first);
				targetMappingOutDepth[tempPair.first] = 0;
				//	deleteList.push_back(tempPair.first);
			}
		}
		if (targetMappingOutDepth[targetNodeID] < searchDepth && targetMappingOutDepth[targetNodeID])
		{
			targetMappingOutSize++;
			targetMappingOut.insert(targetNodeID);
		}
		searchDepth--;

		return;

	}
	bool isCoverQueryGraph()const {
		if (queryGraph.graphSize() == searchDepth)	return true;
		return false;
	};
	MapType getMap() const {
		if (isCoverQueryGraph() == false) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
	/*	 MapType getAuxMap() const {
			return mappingAux;
		}*/
};




