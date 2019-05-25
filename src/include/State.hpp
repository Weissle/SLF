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

using namespace std;


template<typename NodeType, typename EdgeType>
class  State {
public:
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef Graph<NodeType, EdgeType> GraphType;
	typedef const NodeType*   NodeCPointer;
	typedef KVPair<NodeIDType, NodeIDType> MapPair;
	typedef unordered_map<NodeIDType, NodeIDType> MapType;
public:
	State() = default;
	~State() = default;

	virtual vector<MapPair> calCandidatePairs()const {
		return vector<MapPair>
			();
	}
	virtual bool checkCanditatePairIsAddable(const MapPair cp) const { return false; }
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

bool boolForDebug = false;

template<typename NodeType, typename EdgeType>
class StateVF2 : public State<NodeType, EdgeType> {
public:
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef Graph<NodeType, EdgeType> GraphType;
	typedef const NodeType*   NodeCPointer;
	typedef State<NodeType, EdgeType>::MapPair MapPair;
	typedef State<NodeType, EdgeType>::MapType MapType;
	typedef unordered_set<NodeIDType> NodeSetType;

private:
	//private member;
//	NodeIDType 
	MapType mapping, mappingAux; //from query to target
	const GraphType &targetGraph, &queryGraph;
	NodeSetType targetGraphUnmap, targetMappingIn, targetMappingOut,
		queryGraphUnmap, queryMappingIn, queryMappingOut;
	unordered_map<NodeIDType, size_t> targetMappingInDepth, targetMappingOutDepth, queryMappingInDepth, queryMappingOutDepth;
	uint32_t searchDepth;
	bool induceGraph = true;

	void seePairID(const MapPair &cp) {
		cout << cp.getKey() << "  " << cp.getValue() << endl;
	}
private:
	void seeMappingContent(const MapPair &cp)const {
		for (const auto it : mapping) {
			if (it.second) {
				const auto queryNodeID = it.first; //it.getKey()->getID();
				const auto targetNodeID = it.second;
				//			if( (queryNode==0))

				cout << '(' << queryNodeID << "," << targetNodeID << ')' << endl;
				//				boolForDebug = true;
			}
		}
		const auto it = cp;
		cout << '(' << it.getKey() << "," << it.getValue() << ')' << endl;
		cout << endl;

	}
	//private function

	bool setContainNodePointer(const NodeSetType &s, const NodeIDType &nodeID) const {
		return (s.find(nodeID) != s.end());
	}
	bool setNotContainNodePointer(const NodeSetType &s, const NodeIDType &nodeID)const {
		return (s.find(nodeID) == s.end());
	}
	NodeIDType selectNodeToCalCanditates(const NodeSetType &nodeSet)const
	{
		return *nodeSet.begin();
	}
	//same label and target node edges' number should more than query node's.
	bool twoNodesMayMatch(NodeIDType queryNodeID, NodeIDType targetNodeID)const
	{
		const auto &queryNode = queryGraph.getNode(queryNodeID);
		const auto &targetNode = targetGraph.getNode(targetNodeID);
		//	if (queryNodePointer->isSameType(*targetNodePointer) && *targetNodePointer >= *queryNodePointer)return true;
		return ((queryNode.isSameType(targetNode)) && (queryNode <= targetNode));
		//		return false;
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
			//	if (targetGraphUnmap.find(targetTargetNodePointer) == targetGraphUnmap.end()) {
			if (setNotContainNodePointer(targetGraphUnmap, targetTargetNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetTargetNodeID);
				if (tempMappingPair != mapping.end())continue;
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
		//		seePairID(cp);
		const auto &querySourceNodeID = cp.getKey();
		const auto &targetSourceNodeID = cp.getValue();

		const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : querySourceNodePointer->getOutEdges()) {
			//from queryNode to tempNode
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();

			const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			//this tempnode have been mapped
		//	auto temp1 = queryGraphUnmap.find(queryTargetNodePointer);
		//	auto temp2 = queryGraphUnmap.end();
			//have been mapped
		//	if (queryGraphUnmap.find(queryTargetNodePointer) == queryGraphUnmap.end()) {
			if (setNotContainNodePointer(queryGraphUnmap, queryTargetNodeID)) {
				const auto tempMappingPair = mapping.find(querySourceNodeID);
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

		//	const auto &queryTargetNodePointer = cp.getKey();
		//	const auto &targetTargetNodePointer = cp.getValue();
		for (const auto &tempEdge : targetTargetNodePointer->getInEdges()) {
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			//		if (targetGraphUnmap.find(targetSourceNodePointer) == targetGraphUnmap.end()) {
			if (setNotContainNodePointer(targetGraphUnmap, targetSourceNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetSourceNodeID);
				if (tempMappingPair != mapping.end())continue;
				const auto &querySourceNodeID = tempMappingPair->second;
				const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
				//			assert(querySourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
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
			//	if (queryGraphUnmap.find(querySourceNodePointer) == queryGraphUnmap.end()) {
			if (setNotContainNodePointer(queryGraphUnmap, querySourceNodeID)) {
				nodeToMappingNum++;
				const auto tempMappingPair = mapping.find(querySourceNodeID);
				assert((tempMappingPair != mapping.end()) && "mapping not exist the map about this node");
				const auto & targetSourceNodeID = tempMappingPair->second;
				const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
				//				const auto &targetSourceNodePointer = mapping[querySourceNodePointer];
						//		assert(targetSourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
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
			//	if (queryGraphUnmap.find(querySourceNodePointer) != queryGraphUnmap.end())
			if (setContainNodePointer(queryGraphUnmap, querySourceNodeID))
			{
				//		if (queryMappingIn.find(querySourceNodePointer) != queryMappingIn.end()) ++queryInCount;
				if (setContainNodePointer(queryMappingIn, querySourceNodeID)) ++queryInCount;
				else ++queryNotTCount;
			}
			else ++qtemp;

		}
		for (const auto &tempEdge : targetTargetNodePointer->getInEdges())
		{
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			//		if (targetGraphUnmap.find(targetSourceNodePointer) != targetGraphUnmap.end())
			if (setContainNodePointer(targetGraphUnmap, targetSourceNodeID))
			{
				//	if (targetMappingIn.find(targetSourceNodePointer) != targetMappingIn.end()) ++targetInCount;
				if (setContainNodePointer(targetMappingIn, targetSourceNodeID)) ++targetInCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		/*cout << "ic : " << queryInCount << " ntc : " << queryNotTCount << " temp : " << qtemp << " edgenum : " << queryTargetNodePointer->getInEdgesNum() << endl;
		cout << "ic : " << targetInCount << " ntc : " << targetNotTCount << " temp : " << ttemp << " edgenum : " << targetTargetNodePointer->getInEdgesNum() << endl;*/
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
			//	if (queryGraphUnmap.find(queryTargetNodePointer) != queryGraphUnmap.end())
			if (setContainNodePointer(queryGraphUnmap, queryTargetNodeID))
			{
				//		if (queryMappingOut.find(queryTargetNodePointer) != queryMappingOut.end()) ++queryOutCount;
				if (setContainNodePointer(queryMappingOut, queryTargetNodeID)) ++queryOutCount;
				else ++queryNotTCount;
			}
			else ++qtemp;
		}
		for (const auto &tempEdge : targetSourceNodePointer->getOutEdges())
		{
			const auto &targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto & targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			//	if (targetGraphUnmap.find(targetTargetNodePointer) != targetGraphUnmap.end())
			if (setContainNodePointer(targetGraphUnmap, targetTargetNodeID))
			{
				//		if (targetMappingOut.find(targetTargetNodePointer) != targetMappingOut.end()) ++targetOutCount;
				if (setContainNodePointer(targetMappingOut, targetTargetNodeID)) ++targetOutCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		/*	cout << "Query  : oc : " << queryOutCount << " ntc : " << queryNotTCount << " temp : " << qtemp << " edgenum : " << querySourceNodePointer->getOutEdgesNum() << endl;
			cout << "Target : oc : " << targetOutCount << " ntc : " << targetNotTCount << " temp : " << ttemp << " edgenum : " << targetSourceNodePointer->getOutEdgesNum() << endl;
			cout << ((queryOutCount > targetOutCount) ? "False" : "True" )<< endl;*/
		if (queryOutCount > targetOutCount || queryNotTCount > targetNotTCount) return false;
		else return true;
	}
	bool notInOrOutRule(const MapPair &cp)
	{

	}
public:
	StateVF2(const GraphType& _t, const GraphType& _q, bool _induceGraph) :State<NodeType, EdgeType>(), targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {

		auto calASizeForHash = [](const size_t need) {
			size_t i = 16;
			while (i < need) i = i << 1;
			if (i * 0.9 > need) return i;
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
		for (auto &tempNode : targetGraph.getAllNodes()) {
			//	getNodeID(tempNode);
			targetGraphUnmap.insert(NodeType::getNodeID(tempNode));
		}
		for (auto &tempNode : queryGraph.getAllNodes()) {

			queryGraphUnmap.insert(NodeType::getNodeID(tempNode));
		}
		//	targetGraphUnmap.rehash(targetHashSize);

			/*	cout << "target" << endl;
				for (auto it : targetGraphUnmap) cout << it->getID() << endl;

				cout << "query" << endl;
				for (auto it : queryGraphUnmap) cout << it->getID() << endl;*/

	};
	StateVF2() = default;
	~StateVF2() = default;

public:
	//public function
	virtual vector<MapPair> calCandidatePairs()const
	{
		{
			vector<MapPair> answer;
			const NodeSetType * queryNodeToMatchSetPointer, *targetNodeToMatchSetPointer;
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
			//		const auto &queryNodeToMatchPointer = selectNodeToCalCanditates(queryNodeToMatchSet);
			const auto &queryNodeToMatchID = selectNodeToCalCanditates(queryNodeToMatchSet);
			for (const auto &temptargetNodeID : targetNodeToMatchSet) {
				if (twoNodesMayMatch(queryNodeToMatchID, temptargetNodeID)) {
					answer.push_back(MapPair(queryNodeToMatchID, temptargetNodeID));
				}
			}
			return answer;
		}
	}
	virtual bool checkCanditatePairIsAddable(const MapPair &cp)const
	{
		//		bool answer = (sourceRule(cp) && targetRule(cp));/* && inRule(cp) && outRule(cp));*/
		//		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp);
		//		bool answer = sourceRule(cp) && targetRule(cp) && outRule(cp);
		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp);
		return answer;

	}
	virtual void addCanditatePairToMapping(const MapPair &cp)
	{

		mapping[cp.getKey()] = cp.getValue();
		mappingAux[cp.getValue()] = cp.getKey();
		const auto targetNodeID = cp.getValue();
		const auto queryNodeID = cp.getKey();

		targetGraphUnmap.erase(targetNodeID);
		targetMappingIn.erase(targetNodeID);
		targetMappingOut.erase(targetNodeID);
		queryGraphUnmap.erase(queryNodeID);
		queryMappingIn.erase(queryNodeID);
		queryMappingOut.erase(queryNodeID);

		const auto &targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto &queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		for (const auto &tempEdge : targetNodePointer->getInEdges()) {
			const auto &sourceNodeID = tempEdge.getSourceNodeID();
			// was not be mapped
		//	if (targetGraphUnmap.find(tempNodePointer) != targetGraphUnmap.end()) {
			if (setContainNodePointer(targetGraphUnmap, sourceNodeID)) {
				targetMappingIn.insert(sourceNodeID);
				if (targetMappingInDepth[sourceNodeID] == 0) targetMappingInDepth[sourceNodeID] = searchDepth;
			}
		}
		for (const auto &tempEdge : targetNodePointer->getOutEdges()) {
			const auto &targetNodeID = tempEdge.getTargetNodeID();
			if (setContainNodePointer(targetGraphUnmap, targetNodeID)) {
				targetMappingOut.insert(targetNodeID);
				if (targetMappingOutDepth[targetNodeID] == 0) targetMappingOutDepth[targetNodeID] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNodePointer->getOutEdges()) {
			const auto &targetNodeID = tempEdge.getTargetNodeID();

			if (setContainNodePointer(queryGraphUnmap, targetNodeID)) {
				queryMappingOut.insert(targetNodeID);
				if (queryMappingOutDepth[targetNodeID] == 0) queryMappingOutDepth[targetNodeID] = searchDepth;
			}
		}
		for (const auto &tempEdge : queryNodePointer->getInEdges()) {
			const auto &sourceNodeID = tempEdge.getSourceNodeID();
			if (setContainNodePointer(queryGraphUnmap, sourceNodeID)) {
				queryMappingIn.insert(sourceNodeID);
				if (queryMappingInDepth[sourceNodeID] == 0) queryMappingInDepth[sourceNodeID] = searchDepth;
			}
		}

		return;
	}
	virtual void deleteCanditatePairToMapping(const MapPair &cp)
	{
		/*	auto deletePairFunction = [](unordered_map<NodeCPointer, int> &m, vector<NodeCPointer> &v) {
				for (const auto &it : v) {
					m.erase(it);
				}
				v.empty();
			};*/

			/*	mapping[cp.getKey()] = nullptr;
				mappingAux[cp.getValue()] = nullptr;*/
		mapping.erase(cp.getKey());
		mapping.erase(cp.getValue());
		//		mappingAux.erase(cp.getValue());
		const auto &queryNodeID = cp.getKey();
		const auto &targetNodeID = cp.getValue();

		targetGraphUnmap.insert(targetNodeID);
		queryGraphUnmap.insert(queryNodeID);

		vector<NodeCPointer> deleteList;
		for (const auto& tempPair : queryMappingInDepth) {
			if (tempPair.second == searchDepth) {
				queryMappingIn.erase(tempPair.first);
				queryMappingInDepth[tempPair.first] = 0;
				//			deleteList.push_back(tempPair.first);
			}
		}
		if (queryMappingInDepth[queryNodeID] < searchDepth && queryMappingInDepth[queryNodeID]) queryMappingIn.insert(queryNodeID);

		//		deletePairFunction(queryMappingInDepth, deleteList);

		for (const auto& tempPair : queryMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				queryMappingOut.erase(tempPair.first);
				queryMappingOutDepth[tempPair.first] = 0;
				//				deleteList.push_back(tempPair.first);
			}
		}
		if (queryMappingOutDepth[queryNodeID] < searchDepth && queryMappingOutDepth[queryNodeID]) queryMappingOut.insert(queryNodeID);
		//	if (queryMappingInDepth[queryNodeID] < searchDepth ) queryMappingIn.insert(queryNodeID);
			//		deletePairFunction(queryMappingOutDepth, deleteList);
		for (const auto& tempPair : targetMappingInDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingIn.erase(tempPair.first);
				targetMappingInDepth[tempPair.first] = 0;
				//			deleteList.push_back(tempPair.first);
			}
		}
		//	deletePairFunction(targetMappingInDepth, deleteList);
		if (targetMappingInDepth[targetNodeID] < searchDepth && targetMappingInDepth[targetNodeID]) targetMappingIn.insert(targetNodeID);
		//	if (targetMappingInDepth[targetNodeID] < searchDepth ) targetMappingIn.insert(targetNodeID);
		for (const auto& tempPair : targetMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingOut.erase(tempPair.first);
				targetMappingOutDepth[tempPair.first] = 0;
				//	deleteList.push_back(tempPair.first);
			}
		}
		if (targetMappingOutDepth[targetNodeID] < searchDepth && targetMappingOutDepth[targetNodeID]) targetMappingOut.insert(targetNodeID);
		//	if (targetMappingOutDepth[targetNodeID] < searchDepth ) targetMappingOut.insert(targetNodeID);
			//	deletePairFunction(targetMappingOutDepth, deleteList);
		searchDepth--;

		return;

	}
	virtual bool isCoverQueryGraph()const {
		if (queryGraph.graphSize() == searchDepth) {
			return true;
		}
		return false;
	};
	virtual MapType getMap() const {
		if (isCoverQueryGraph() == false) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
	virtual MapType getAuxMap() const {
		return mappingAux;
	}
};




