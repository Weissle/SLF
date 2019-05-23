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
	typedef KVPair<NodeCPointer, NodeCPointer> MapPair;
	typedef unordered_map<NodeCPointer, NodeCPointer> MapType;
public:
	State() = default;
	~State() = default;

	virtual vector<MapPair> calCandidatePairs() {
		return vector<MapPair>
			();
	}
	virtual bool checkCanditatePairIsAddable (const MapPair cp) { return false; }
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
	typedef KVPair<NodeCPointer, NodeCPointer> MapPair;
	//	typedef unordered_set<MapPair> MapType;
	typedef unordered_map<NodeCPointer, NodeCPointer> MapType;
private:
	//private member;

	MapType mapping, mappingAux; //from query to target
	const GraphType &targetGraph, &queryGraph;
	unordered_set<NodeCPointer> targetGraphUnmap, targetMappingIn, targetMappingOut,
		queryGraphUnmap, queryMappingIn, queryMappingOut;
	unordered_map<NodeCPointer, int> targetMappingInDepth, targetMappingOutDepth, queryMappingInDepth, queryMappingOutDepth;
	uint32_t searchDepth;
	bool induceGraph = true;

	void seePairID(const MapPair &cp) {
		cout << cp.getKey()->getID() << "  " << cp.getValue()->getID() << endl;
	}
private:

	//private function


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


	bool sourceRuleInduceSubgraph(const MapPair &cp, const int need) {
	/*	const auto &targetSourceNode = cp.getValue();
		int have = 0;
		for (const auto &it : targetSourceNode->getOutEdges()) {
			const auto targetTargetNodeID = it.getTargetNodeID();
			const auto targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			if (targetGraphUnmap.find(targetTargetNodePointer) == targetGraphUnmap.end()) {
				//in the mapping
				++have;
			}
		}
		return have == need;*/
		const auto &querySourceNodePointer = cp.getKey();
		const auto &targetSourceNodePointer = cp.getValue();
	/*	
		for (auto it : mapping) {
	/*		auto queryNodeID = it.getKey()->getID();
			auto targetNodeID = it.getValue()->getID();
			if( (queryNode==0))
			if (it.second) {
				cout << '(' << it.first->getID() << "," << it.second->getID() << ')' << endl;
//				boolForDebug = true;
			}
		}
		cout << endl;
		*/
	
		for (const auto &tempEdge : targetSourceNodePointer->getOutEdges()) {
			const auto &targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto &targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			if (targetGraphUnmap.find(targetTargetNodePointer) == targetGraphUnmap.end()) {
				const auto &queryTargetNodePointer = mappingAux[targetTargetNodePointer];
				assert(queryTargetNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if(querySourceNodePointer->existSameTypeEdgeToNode(*queryTargetNodePointer, tempEdge) == false) return false;
			}
		}
		return true;

	}
	bool sourceRule(const MapPair &cp)
	{
//		seePairID(cp);
		const auto &querySourceNodePointer = cp.getKey();
		const auto &targetSourceNodePointer = cp.getValue();
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : querySourceNodePointer->getOutEdges()) {
			//from queryNode to tempNode
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();

			const auto &queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			//this tempnode have been mapped
			auto temp1 = queryGraphUnmap.find(queryTargetNodePointer);
			auto temp2 = queryGraphUnmap.end();
			//have been mapped
			if (queryGraphUnmap.find(queryTargetNodePointer) == queryGraphUnmap.end()) {

				const auto &targetTargetNodePointer = mapping[queryTargetNodePointer];
				assert(targetTargetNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (targetSourceNodePointer->existSameTypeEdgeToNode(*targetTargetNodePointer, tempEdge) == false) return false;
				++nodeToMappingNum;
			}
		}
		if (induceGraph) return sourceRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}
	bool targetRuleInduceSubgraph(const MapPair &cp, const int need) {
	/*	const auto &targetSourceNode = cp.getValue();
		int have = 0;
		for (const auto &it : targetSourceNode->getInEdges()) {
			const auto targetSourceNodeID = it.getSourceNodeID();
			const auto targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (targetGraphUnmap.find(targetSourceNodePointer) == targetGraphUnmap.end()) {
				//in the mapping
				++have;
			}
		}
		return have == need;*/
		const auto &queryTargetNodePointer = cp.getKey();
		const auto &targetTargetNodePointer = cp.getValue();
		for (const auto &tempEdge : targetTargetNodePointer->getInEdges()) {
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (targetGraphUnmap.find(targetSourceNodePointer) == targetGraphUnmap.end()) {
				const auto &querySourceNodePointer = mappingAux[targetSourceNodePointer];
				assert(querySourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (queryTargetNodePointer->existSameTypeEdgeFromNode(*querySourceNodePointer, tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool targetRule(const MapPair &cp)
	{
		const auto &queryTargetNodePointer = cp.getKey();
		const auto &targetTargetNodePointer = cp.getValue();
		int nodeToMappingNum = 0;
		for (const auto &tempEdge : queryTargetNodePointer->getInEdges()) {
			const auto &querySourceNodeID = tempEdge.getSourceNodeID();
			const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
			if (queryGraphUnmap.find(querySourceNodePointer) == queryGraphUnmap.end()) {
				nodeToMappingNum++;
				const auto &targetSourceNodePointer = mapping[querySourceNodePointer];
				assert(targetSourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (targetTargetNodePointer->existSameTypeEdgeFromNode(*targetSourceNodePointer, tempEdge) == false) return false;
			}
		}
		if (induceGraph) return targetRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}

	bool inRule(const MapPair &cp)
	{
		const auto &queryTargetNodePointer = cp.getKey();
		const auto &targetTargetNodePointer = cp.getValue();
		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0;
		size_t qtemp = 0, ttemp = 0;
		for (const auto &tempEdge : queryTargetNodePointer->getInEdges())
		{
			const auto &querySourceNodeID = tempEdge.getSourceNodeID();
			const auto &querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
			if (queryGraphUnmap.find(querySourceNodePointer) != queryGraphUnmap.end())
			{
				if (queryMappingIn.find(querySourceNodePointer) != queryMappingIn.end()) ++queryInCount;
				else ++queryNotTCount;
			}
			else ++qtemp;

		}
		for (const auto &tempEdge : targetTargetNodePointer->getInEdges())
		{
			const auto &targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto &targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (targetGraphUnmap.find(targetSourceNodePointer) != targetGraphUnmap.end())
			{
				if (targetMappingIn.find(targetSourceNodePointer) != targetMappingIn.end()) ++targetInCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		/*cout << "ic : " << queryInCount << " ntc : " << queryNotTCount << " temp : " << qtemp << " edgenum : " << queryTargetNodePointer->getInEdgesNum() << endl;
		cout << "ic : " << targetInCount << " ntc : " << targetNotTCount << " temp : " << ttemp << " edgenum : " << targetTargetNodePointer->getInEdgesNum() << endl;*/
		if (queryInCount > targetInCount /*|| queryNotTCount > targetNotTCount*/) return false;
		else return true;
	}
	bool outRule(const MapPair &cp)
	{
		const auto &queryNodePointer = cp.getKey();
		const auto &targetNodePointer = cp.getValue();
		size_t queryOutCount = 0, targetOutCount = 0, queryNotTCount = 0, targetNotTCount = 0;
		size_t qtemp = 0, ttemp = 0;
		for (const auto &tempEdge : queryNodePointer->getOutEdges())
		{
			const auto &queryTargetNodeID = tempEdge.getTargetNodeID();
			const auto & queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			if (queryGraphUnmap.find(queryTargetNodePointer) != queryGraphUnmap.end())
			{
				if (queryMappingOut.find(queryTargetNodePointer) != queryMappingOut.end()) ++queryOutCount;
				else ++queryNotTCount;
			}
			else ++qtemp;
		}
		for (const auto &tempEdge : targetNodePointer->getOutEdges())
		{
			const auto &targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto & targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
			if (targetGraphUnmap.find(targetTargetNodePointer) != targetGraphUnmap.end())
			{
				if (targetMappingOut.find(targetTargetNodePointer) != targetMappingOut.end()) ++targetOutCount;
				else ++targetNotTCount;
			}
			else ++ttemp;
		}
		/*	cout << "Query  : oc : " << queryOutCount << " ntc : " << queryNotTCount << " temp : " << qtemp << " edgenum : " << queryNodePointer->getOutEdgesNum() << endl;
			cout << "Target : oc : " << targetOutCount << " ntc : " << targetNotTCount << " temp : " << ttemp << " edgenum : " << targetNodePointer->getOutEdgesNum() << endl;
			cout << ((queryOutCount > targetOutCount) ? "False" : "True" )<< endl;*/
		if (queryOutCount > targetOutCount/* || queryNotTCount > targetNotTCount*/) return false;
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

			targetGraphUnmap.insert(&tempNode);
		}
		for (auto &tempNode : queryGraph.getAllNodes()) {
			queryGraphUnmap.insert(&tempNode);
		}
		targetGraphUnmap.rehash(targetHashSize);

	/*	cout << "target" << endl;
		for (auto it : targetGraphUnmap) cout << it->getID() << endl;

		cout << "query" << endl;
		for (auto it : queryGraphUnmap) cout << it->getID() << endl;*/

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
		bool answer = (sourceRule(cp) && targetRule(cp));/* && inRule(cp) && outRule(cp));*/
//		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp);
//		bool answer = sourceRule(cp) && targetRule(cp) && outRule(cp);
//		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp);
		return answer;

	}
	virtual void addCanditatePairToMapping(const MapPair &cp)
	{

		mapping[cp.getKey()] = cp.getValue();
		mappingAux[cp.getValue()] = cp.getKey();
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
		/*	auto deletePairFunction = [](unordered_map<NodeCPointer, int> &m, vector<NodeCPointer> &v) {
				for (const auto &it : v) {
					m.erase(it);
				}
				v.empty();
			};*/

		mapping[cp.getKey()] = nullptr;
		mappingAux[cp.getValue()] = nullptr;
//		mappingAux.erase(cp.getValue());
		auto queryNodePointer = cp.getKey();
		auto targetNodePointer = cp.getValue();

		targetGraphUnmap.insert(targetNodePointer);
		queryGraphUnmap.insert(queryNodePointer);

		vector<NodeCPointer> deleteList;
		for (const auto& tempPair : queryMappingInDepth) {
			if (tempPair.second == searchDepth) {
				queryMappingIn.erase(tempPair.first);
				queryMappingInDepth[tempPair.first] = 0;
				//			deleteList.push_back(tempPair.first);
			}
		}
		if (queryMappingInDepth[queryNodePointer] < searchDepth) queryMappingIn.insert(queryNodePointer);
		//		deletePairFunction(queryMappingInDepth, deleteList);

		for (const auto& tempPair : queryMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				queryMappingOut.erase(tempPair.first);
				queryMappingOutDepth[tempPair.first] = 0;
				//				deleteList.push_back(tempPair.first);
			}
		}
		if (queryMappingOutDepth[queryNodePointer] < searchDepth) queryMappingOut.insert(queryNodePointer);
		//		deletePairFunction(queryMappingOutDepth, deleteList);
		for (const auto& tempPair : targetMappingInDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingIn.erase(tempPair.first);
				targetMappingInDepth[tempPair.first] = 0;
				//			deleteList.push_back(tempPair.first);
			}
		}
		//	deletePairFunction(targetMappingInDepth, deleteList);
		if (targetMappingInDepth[targetNodePointer] < searchDepth) targetMappingIn.insert(targetNodePointer);
		for (const auto& tempPair : targetMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingOut.erase(tempPair.first);
				targetMappingOutDepth[tempPair.first] = 0;
				//	deleteList.push_back(tempPair.first);
			}
		}
		if (targetMappingOutDepth[targetNodePointer] < searchDepth) targetMappingOut.insert(targetNodePointer);
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




