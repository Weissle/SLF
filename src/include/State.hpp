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
#include<queue>

using namespace std;


template<typename GraphType>
class  State {
public:
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;


	typedef const NodeType* NodeCPointer;
	typedef KVPair<NodeIDType, NodeIDType> MapPair;
	typedef unordered_map<NodeIDType, NodeIDType> MapType;
public:
	State() = default;
	~State() = default;

	virtual vector<MapPair> calCandidatePairs()const = 0;
	virtual bool checkCanditatePairIsAddable(const MapPair& cp) const = 0;
	virtual void addCanditatePairToMapping(const MapPair& cp) = 0;
	virtual void deleteCanditatePairToMapping(const MapPair& cp) = 0;
	virtual bool isCoverQueryGraph()const = 0;
	virtual MapType getMap(bool showNotCoverWarning = true) const = 0;

};


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
	typedef const NodeType* NodeCPointer;
	typedef StateBase::MapPair MapPair;
	typedef StateBase::MapType MapType;

	typedef unordered_set<NodeIDType> NodeSetType;

private:

	MapType mapping, mappingAux; //from query to target
	const GraphType& targetGraph, & queryGraph;
	NodeSetType targetGraphUnmap, targetMappingIn, targetMappingOut, targetMappingBoth,
		queryGraphUnmap, queryMappingIn, queryMappingOut, queryMappingBoth;
	size_t targetMappingInSize = 0, targetMappingOutSize = 0, queryMappingInSize = 0,
		queryMappingOutSize = 0, targetBothInOutSize = 0, queryBothInOutSize = 0;

	unordered_map<NodeIDType, size_t> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;
	size_t searchDepth;

	vector<NodeIDType> matchSequence;
	bool induceGraph = true;

	void seePairID(const MapPair& cp)const {
		cout << "Ready To Mapping" << "(" << cp.getKey() << "," << cp.getValue() << ")" << endl;
	}
	void seeMappingContent()const {
		for (const auto it : mapping) {
			if (it.second) {
				const auto queryNodeID = it.first;
				const auto targetNodeID = it.second;
				cout << '(' << queryNodeID << "," << targetNodeID << ')' << endl;

			}
		}
	}

	bool setContainNodeID(const NodeSetType& s, const NodeIDType& nodeID) const {
		return (s.find(nodeID) != s.end());
	}
	bool setNotContainNodeID(const NodeSetType & s, const NodeIDType & nodeID)const {
		return (s.find(nodeID) == s.end());
	}
	NodeIDType selectNodeToCalCanditates()const
	{
		return matchSequence[searchDepth];
	}
	//same label and target node edges' number should more than query node's.
	bool twoNodesMayMatch(NodeIDType queryNodeID, NodeIDType targetNodeID)const
	{
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);
		return ((queryNode.isSameType(targetNode)) && (queryNode <= targetNode));

	}

	bool sourceRuleInduceSubgraph(const MapPair & cp, const int need)const {
		const auto& querySourceNodeID = cp.getKey();
		const auto& targetSourceNodeID = cp.getValue();

		const auto& querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto& targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);



		for (const auto& tempEdge : targetSourceNodePointer->getOutEdges()) {
			const auto& targetTargetNodeID = tempEdge.getTargetNodeID();

			if (setNotContainNodeID(targetGraphUnmap, targetTargetNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetTargetNodeID);
				assert((tempMappingPair != mappingAux.end()) && "mapping not exist the map about this node");
				const auto & queryTargetNodeID = tempMappingPair->second;
				const auto & queryTargetNode = queryGraph.getNode(queryTargetNodeID);
				if (querySourceNodePointer->existSameTypeEdgeToNode(queryTargetNode, tempEdge) == false) return false;
			}
		}
		return true;

	}
	bool sourceRule(const MapPair & cp) const
	{
		const auto& querySourceNodeID = cp.getKey();
		const auto& targetSourceNodeID = cp.getValue();

		const auto& querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto& targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
		int nodeToMappingNum = 0;
		for (const auto& tempEdge : querySourceNodePointer->getOutEdges()) {
			const auto& queryTargetNodeID = tempEdge.getTargetNodeID();
			const auto& queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
			//this tempnode have been mapped
			if (setNotContainNodeID(queryGraphUnmap, queryTargetNodeID)) {
				const auto tempMappingPair = mapping.find(queryTargetNodeID);
				assert((tempMappingPair != mapping.end()) && "mapping not exist the map about this node");
				const auto & targetTargetNodeID = tempMappingPair->second;
				const auto & targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);
				if (targetSourceNodePointer->existSameTypeEdgeToNode(*targetTargetNodePointer, tempEdge) == false) return false;
				++nodeToMappingNum;
			}
		}
		if (induceGraph) return sourceRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}
	bool targetRuleInduceSubgraph(const MapPair & cp, const int need)const {
		const auto& queryTargetNodeID = cp.getKey();
		const auto& targetTargetNodeID = cp.getValue();

		const auto& queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto& targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);


		for (const auto& tempEdge : targetTargetNodePointer->getInEdges()) {
			const auto& targetSourceNodeID = tempEdge.getSourceNodeID();


			if (setNotContainNodeID(targetGraphUnmap, targetSourceNodeID)) {
				const auto tempMappingPair = mappingAux.find(targetSourceNodeID);

				assert((tempMappingPair != mappingAux.end()) && "mapping not exist the map about this node");
				const auto & querySourceNodeID = tempMappingPair->second;
				const auto & querySourceNode = queryGraph.getNode(querySourceNodeID);
				if (queryTargetNodePointer->existSameTypeEdgeFromNode(querySourceNode, tempEdge) == false) return false;
			}
		}
		return true;
	}
	bool targetRule(const MapPair & cp)const
	{
		const auto& queryTargetNodeID = cp.getKey();
		const auto& targetTargetNodeID = cp.getValue();

		const auto& queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto& targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

		int nodeToMappingNum = 0;
		for (const auto& tempEdge : queryTargetNodePointer->getInEdges()) {
			const auto& querySourceNodeID = tempEdge.getSourceNodeID();
			const auto& querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);

			if (setNotContainNodeID(queryGraphUnmap, querySourceNodeID)) {
				nodeToMappingNum++;
				const auto tempMappingPair = mapping.find(querySourceNodeID);

				assert((tempMappingPair != mapping.end()) && "mapping not exist the map about this node");
				const auto & targetSourceNodeID = tempMappingPair->second;
				const auto & targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
				assert(targetSourceNodePointer != nullptr && "map a nullptr ? means this node mappes nothing but is regarded have been mapped. ");
				if (targetTargetNodePointer->existSameTypeEdgeFromNode(*targetSourceNodePointer, tempEdge) == false) return false;
			}
		}
		if (induceGraph) return targetRuleInduceSubgraph(cp, nodeToMappingNum);
		else return true;
	}

	bool inRule(const MapPair & cp)const
	{
		const auto& queryTargetNodeID = cp.getKey();
		const auto& targetTargetNodeID = cp.getValue();

		const auto& queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);
		const auto& targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0, queryOutCount = 0, targetOutCount = 0, queryBothCount = 0, targetBothCount = 0;
		//	size_t qtemp = 0, ttemp = 0;
		for (const auto& tempEdge : queryTargetNodePointer->getInEdges())
		{
			const auto& querySourceNodeID = tempEdge.getSourceNodeID();
			const auto& querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
			if (setContainNodeID(queryGraphUnmap, querySourceNodeID))
			{
				/*	if (setContainNodeID(queryMappingIn, querySourceNodeID)) ++queryInCount;
					if (setContainNodeID(queryMappingOut, querySourceNodeID))++queryOutCount;
					if (setContainNodeID(queryMappingBoth, querySourceNodeID))++queryBothCount;
					else ++queryNotTCount;*/
				const bool o = setContainNodeID(queryMappingOut, querySourceNodeID);
				const bool i = setContainNodeID(queryMappingIn, querySourceNodeID);
				const bool b = setContainNodeID(queryMappingBoth, querySourceNodeID);
				assert((o && i) == b && "error happened");
				if (o) ++queryOutCount;
				if (i) ++queryInCount;
				if (b) ++queryBothCount;
				if (!b && !i && !o) ++queryNotTCount;
			}
			//			else ++qtemp;

		}
		for (const auto& tempEdge : targetTargetNodePointer->getInEdges())
		{
			const auto& targetSourceNodeID = tempEdge.getSourceNodeID();
			const auto& targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);
			if (setContainNodeID(targetGraphUnmap, targetSourceNodeID))
			{
				/*	if (setContainNodeID(targetMappingIn, targetSourceNodeID)) ++targetInCount;
					if (setContainNodeID(targetMappingOut, targetSourceNodeID)) ++targetOutCount;
					if (setContainNodeID(targetMappingBoth, targetSourceNodeID))++targetBothCount;*/
				const bool o = setContainNodeID(targetMappingOut, targetSourceNodeID);
				const bool i = setContainNodeID(targetMappingIn, targetSourceNodeID);
				const bool b = setContainNodeID(targetMappingBoth, targetSourceNodeID);
				assert((o && i) == b && "error happened");
				if (o) ++targetOutCount;
				if (i) ++targetInCount;
				if (b) ++targetBothCount;
				if (!b && !i && !o) ++targetNotTCount;
			}
			//		else ++ttemp;
		}
		if (queryInCount > targetInCount || queryNotTCount > targetNotTCount || queryOutCount > targetOutCount || queryBothCount > targetBothCount) return false;
		if ((queryInCount - queryBothCount) > (targetInCount - targetBothCount) || (queryOutCount - queryBothCount) > (targetOutCount - targetBothCount)) return false;
		//	if (  (queryInCount))
		return true;
	}
	bool outRule(const MapPair & cp)const
	{

		const auto& querySourceNodeID = cp.getKey();
		const auto& targetSourceNodeID = cp.getValue();

		const auto& querySourceNodePointer = queryGraph.getNodePointer(querySourceNodeID);
		const auto& targetSourceNodePointer = targetGraph.getNodePointer(targetSourceNodeID);


		//	size_t queryOutCount = 0, targetOutCount = 0, queryNotTCount = 0, targetNotTCount = 0;
		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0, queryOutCount = 0, targetOutCount = 0, queryBothCount = 0, targetBothCount = 0;
		//	size_t qtemp = 0, ttemp = 0;
		for (const auto& tempEdge : querySourceNodePointer->getOutEdges())
		{
			const auto& queryTargetNodeID = tempEdge.getTargetNodeID();
			const auto& queryTargetNodePointer = queryGraph.getNodePointer(queryTargetNodeID);

			if (setContainNodeID(queryGraphUnmap, queryTargetNodeID))
			{
				const bool o = setContainNodeID(queryMappingOut, queryTargetNodeID);
				const bool i = setContainNodeID(queryMappingIn, queryTargetNodeID);
				const bool b = setContainNodeID(queryMappingBoth, queryTargetNodeID);

				if (o) ++queryOutCount;
				if (i) ++queryInCount;
				if (b)++queryBothCount;
				if (!b && !i && !o) ++queryNotTCount;
			}
			//		else ++qtemp;
		}
		for (const auto& tempEdge : targetSourceNodePointer->getOutEdges())
		{
			const auto& targetTargetNodeID = tempEdge.getTargetNodeID();
			const auto& targetTargetNodePointer = targetGraph.getNodePointer(targetTargetNodeID);

			if (setContainNodeID(targetGraphUnmap, targetTargetNodeID))
			{
				const bool o = setContainNodeID(targetMappingOut, targetTargetNodeID);
				const bool i = setContainNodeID(targetMappingIn, targetTargetNodeID);
				const bool b = setContainNodeID(targetMappingBoth, targetTargetNodeID);
				if (o) ++targetOutCount;
				if (i) ++targetInCount;
				if (b)++targetBothCount;

				if (!b && !i && !o) ++targetNotTCount;
			}
			//		else ++ttemp;
		}

		if (queryInCount > targetInCount || queryNotTCount > targetNotTCount || queryOutCount > targetOutCount || queryBothCount > targetBothCount) return false;
		if ((queryInCount - queryBothCount) > (targetInCount - targetBothCount) || (queryOutCount - queryBothCount) > (targetOutCount - targetBothCount)) return false;
		return true;
	}
	bool notInOrOutRule(const MapPair & cp)const
	{

	}
#define FUN1

	void seleteMatchOrder() {

		NodeSetType nodeNotInMatchSet = queryGraphUnmap;
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		unordered_map<NodeIDType, char> map;
		const auto calNodeMatchPoint = [=](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum();
#ifdef FUN1
			for (const auto& tempEdge : node.getInEdges()) if (nodeNotInMatchSet.find(tempEdge.getSourceNodeID()) == nodeNotInMatchSet.end()) p1 += 1;
			for (const auto& tempEdge : node.getOutEdges()) if (nodeNotInMatchSet.find(tempEdge.getTargetNodeID()) == nodeNotInMatchSet.end()) p1 += 1;
#endif
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [=]() {
			double nodePoint = -1;
			NodeCPointer answer;
			for (const auto& tempNodeID : nodeNotInMatchSet) {
				const auto& tempNode = queryGraph.getNode(tempNodeID);
				const auto tempPoint = calNodeMatchPoint(tempNode);
				if (tempPoint > nodePoint) {
					answer = &tempNode;
					nodePoint = tempPoint;
				}
			}
			return NodeMatchPointPair(answer, nodePoint);
		};

		matchSequence.reserve(queryGraph.graphSize() + 5);


		const auto cmp = [](const NodeMatchPointPair & a, const NodeMatchPointPair & b) {
			return a.getValue() < b.getValue();
		};
		priority_queue<NodeMatchPointPair, vector<NodeMatchPointPair>, decltype(cmp)> order(cmp);



		while (nodeNotInMatchSet.empty() == false) {
			const auto nodePointPair = seleteAGoodNodeToMatch();
			nodeNotInMatchSet.erase(nodePointPair.getKey()->getID());
			order.push(seleteAGoodNodeToMatch());
			while (order.empty() == false) {

				const auto nodePair = order.top();
				order.pop();
				const auto& nodePointer = nodePair.getKey();
				const auto& nodeID = nodePointer->getID();
				nodeNotInMatchSet.erase(nodeID);
				matchSequence.push_back(nodeID);
				for (const auto& tempEdge : nodePointer->getInEdges()) {
					if (setContainNodeID(nodeNotInMatchSet, tempEdge.getSourceNodeID())) {
						const auto& sourceNodePointer = queryGraph.getNodePointer(tempEdge.getSourceNodeID());
						nodeNotInMatchSet.erase(tempEdge.getSourceNodeID());
						NodeMatchPointPair tempPair(sourceNodePointer, calNodeMatchPoint(*sourceNodePointer));
						order.push(tempPair);
					}
				}
				for (const auto& tempEdge : nodePointer->getOutEdges()) {
					if (setContainNodeID(nodeNotInMatchSet, tempEdge.getTargetNodeID())) {
						const auto& targetNodePointer = queryGraph.getNodePointer(tempEdge.getTargetNodeID());
						nodeNotInMatchSet.erase(tempEdge.getTargetNodeID());
						NodeMatchPointPair tempPair(targetNodePointer, calNodeMatchPoint(*targetNodePointer));
						order.push(tempPair);
					}
				}
			}

		}
		/*	for (const auto &tempNodeID : matchSequence) {
				cout << tempNodeID << " : " << calNodeMatchPoint(queryGraph.getNode(tempNodeID)) << endl;;
			}*/


	}

public:
	StateVF2(const GraphType & _t, const GraphType & _q, bool _induceGraph) :targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {

		auto calASizeForHash = [](const size_t need) {
			size_t i = 16;
			while (i < need) i = i << 1;
			if (i * 0.6 > need) return i;
			else return i << 1;
		};
		const int queryHashSize = calASizeForHash(queryGraph.graphSize());
		const int targetHashSize = calASizeForHash(targetGraph.graphSize());
		mapping.reserve(queryHashSize);
		mappingAux.reserve(targetHashSize);
		targetMappingIn.reserve(targetHashSize);
		targetMappingOut.reserve(targetHashSize);
		targetMappingBoth.reserve(targetHashSize);
		targetGraphUnmap.reserve(targetHashSize);

		queryMappingIn.reserve(queryHashSize);
		queryMappingOut.reserve(queryHashSize);
		queryMappingBoth.reserve(queryHashSize);
		queryGraphUnmap.reserve(queryHashSize);

		targetMappingInDepth.reserve(targetHashSize);
		targetMappingOutDepth.reserve(targetHashSize);
		queryMappingInDepth.reserve(queryHashSize);
		queryMappingOutDepth.reserve(queryHashSize);

		searchDepth = 0;
		for (auto& tempNode : targetGraph.getAllNodes())
		{
			targetGraphUnmap.insert(getNodeID(tempNode));
		}
		for (auto& tempNode : queryGraph.getAllNodes())
		{
			queryGraphUnmap.insert(getNodeID(tempNode));
		}
		seleteMatchOrder();


	};
	StateVF2() = default;
	~StateVF2() = default;

public:
	//public function
	vector<MapPair> calCandidatePairs()const
	{
		vector<MapPair> answer;

		const bool numOfInOut = (queryMappingInSize <= targetMappingInSize)
			&& (queryMappingOutSize <= targetMappingOutSize) && (queryBothInOutSize <= targetBothInOutSize);
		if (numOfInOut == false)return answer;

		const auto & queryNodeToMatchID = selectNodeToCalCanditates();
		const bool queryNodeInIn = setContainNodeID(queryMappingIn, queryNodeToMatchID);
		const bool queryNodeInOut = setContainNodeID(queryMappingOut, queryNodeToMatchID);
		const NodeSetType * tempNodeSetPointer;
		if (queryNodeInIn && queryNodeInOut) tempNodeSetPointer = (queryMappingInSize > queryMappingOutSize) ? &targetMappingOut : &targetMappingIn;
		else if (queryNodeInIn) tempNodeSetPointer = &targetMappingIn;
		else if (queryNodeInOut)tempNodeSetPointer = &targetMappingOut;
		else tempNodeSetPointer = &targetGraphUnmap;

		answer.reserve(targetMappingInSize + targetMappingOutSize);
		const auto & targetNodeToMatchSet = *tempNodeSetPointer;
		for (const auto& targetNodeToMatchID : targetNodeToMatchSet) {
			if (twoNodesMayMatch(queryNodeToMatchID, targetNodeToMatchID) == false)continue;
			const bool targetNodeInIn = setContainNodeID(targetMappingIn, targetNodeToMatchID);
			if (targetNodeInIn != queryNodeInIn) continue;
			const bool targetNodeInOut = setContainNodeID(targetMappingOut, targetNodeToMatchID);
			if (targetNodeInOut != queryNodeInOut)continue;

			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));

		}
		return answer;

	}
	bool checkCanditatePairIsAddable(const MapPair & cp)const
	{
		//	seePairID(cp);
		//	seeMappingContent(cp);
			//		bool answer = (sourceRule(cp) && targetRule(cp));/* && inRule(cp) && outRule(cp));*/
			//		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp);
			//		bool answer = sourceRule(cp) && targetRule(cp) && outRule(cp);

		bool answer = sourceRule(cp) && targetRule(cp) && inRule(cp) && outRule(cp);
		return answer;

	}
	void addCanditatePairToMapping(const MapPair & cp)
	{

		mapping[cp.getKey()] = cp.getValue();
		mappingAux[cp.getValue()] = cp.getKey();
		const auto targetNodeID = cp.getValue();
		const auto queryNodeID = cp.getKey();

		targetGraphUnmap.erase(targetNodeID);
		queryGraphUnmap.erase(queryNodeID);


		const auto calSetSizeAfterErase = [=](NodeSetType & inSet, NodeSetType & outSet, NodeSetType & bothSet, const NodeIDType & nodeID,
			size_t & inSize, size_t & outSize, size_t & bothSize)
		{
			static size_t findTime = 0;
			bool inInSet = (inSet.find(nodeID) != inSet.end());
			bool inOutSet = (outSet.find(nodeID) != outSet.end());

			if (inInSet) {
				--inSize;
				inSet.erase(nodeID);
			}
			if (inOutSet) {
				--outSize;
				outSet.erase(nodeID);
			}
			if (inInSet && inOutSet) {
				bothSet.erase(nodeID);
				--bothSize;
			}
			return;
		};

		calSetSizeAfterErase(targetMappingIn, targetMappingOut, targetMappingBoth, targetNodeID, targetMappingInSize, targetMappingOutSize, targetBothInOutSize);
		calSetSizeAfterErase(queryMappingIn, queryMappingOut, queryMappingBoth, queryNodeID, queryMappingInSize, queryMappingOutSize, queryBothInOutSize);


		const auto& targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		const auto addOneIfExist = [](const NodeSetType & set, NodeSetType & both, const NodeIDType nodeID, size_t & size) {
			if (set.find(nodeID) != set.end()) {
				++size;
				both.insert(nodeID);
			}
		};

		for (const auto& tempEdge : targetNodePointer->getInEdges()) {
			const auto& sourceNodeID = tempEdge.getSourceNodeID();
			// was not be mapped
			if (setContainNodeID(targetGraphUnmap, sourceNodeID)) {
				targetMappingIn.insert(sourceNodeID);
				if (targetMappingInDepth[sourceNodeID] == 0) {
					targetMappingInDepth[sourceNodeID] = searchDepth;
					++targetMappingInSize;
					addOneIfExist(targetMappingOut, targetMappingBoth, sourceNodeID, targetBothInOutSize);
				}
			}
		}
		for (const auto& tempEdge : targetNodePointer->getOutEdges()) {
			const auto& targetNodeID = tempEdge.getTargetNodeID();
			if (setContainNodeID(targetGraphUnmap, targetNodeID)) {
				targetMappingOut.insert(targetNodeID);
				if (targetMappingOutDepth[targetNodeID] == 0) {
					targetMappingOutDepth[targetNodeID] = searchDepth;
					++targetMappingOutSize;
					addOneIfExist(targetMappingIn, targetMappingBoth, targetNodeID, targetBothInOutSize);
				}
			}
		}
		for (const auto& tempEdge : queryNodePointer->getOutEdges()) {
			const auto& targetNodeID = tempEdge.getTargetNodeID();

			if (setContainNodeID(queryGraphUnmap, targetNodeID)) {
				queryMappingOut.insert(targetNodeID);
				if (queryMappingOutDepth[targetNodeID] == 0) {
					queryMappingOutDepth[targetNodeID] = searchDepth;
					++queryMappingOutSize;
					addOneIfExist(queryMappingIn, queryMappingBoth, targetNodeID, queryBothInOutSize);
				}
			}
		}
		for (const auto& tempEdge : queryNodePointer->getInEdges()) {
			const auto& sourceNodeID = tempEdge.getSourceNodeID();
			if (setContainNodeID(queryGraphUnmap, sourceNodeID)) {
				queryMappingIn.insert(sourceNodeID);
				if (queryMappingInDepth[sourceNodeID] == 0) {
					queryMappingInDepth[sourceNodeID] = searchDepth;
					++queryMappingInSize;
					addOneIfExist(queryMappingOut, queryMappingBoth, sourceNodeID, queryBothInOutSize);
				}
			}
		}

		return;
	}
	void deleteCanditatePairToMapping(const MapPair & cp)
	{

		mapping.erase(cp.getKey());
		mappingAux.erase(cp.getValue());
		const auto& queryNodeID = cp.getKey();
		const auto& targetNodeID = cp.getValue();

		targetGraphUnmap.insert(targetNodeID);
		queryGraphUnmap.insert(queryNodeID);

		for (const auto& tempPair : queryMappingInDepth) {
			if (tempPair.second == searchDepth) {
				--queryMappingInSize;
				queryMappingIn.erase(tempPair.first);
				if (setContainNodeID(queryMappingOut, tempPair.first)) {
					queryMappingBoth.erase(tempPair.first);
					--queryBothInOutSize;
				}

				queryMappingInDepth[tempPair.first] = 0;

			}
		}
		for (const auto& tempPair : queryMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				--queryMappingOutSize;
				if (setContainNodeID(queryMappingIn, tempPair.first)) {
					--queryBothInOutSize;
					queryMappingBoth.erase(tempPair.first);
				}
				queryMappingOut.erase(tempPair.first);
				queryMappingOutDepth[tempPair.first] = 0;

			}
		}

		if (queryMappingInDepth[queryNodeID] < searchDepth && queryMappingInDepth[queryNodeID]) {
			++queryMappingInSize;
			if (setContainNodeID(queryMappingOut, queryNodeID)) {
				queryMappingBoth.insert(queryNodeID);
				++queryBothInOutSize;
			}
			queryMappingIn.insert(queryNodeID);
		}
		if (queryMappingOutDepth[queryNodeID] < searchDepth && queryMappingOutDepth[queryNodeID])
		{
			++queryMappingOutSize;
			if (setContainNodeID(queryMappingIn, queryNodeID)) {
				queryMappingBoth.insert(queryNodeID);
				++queryBothInOutSize;
			}
			queryMappingOut.insert(queryNodeID);
		}


		for (const auto& tempPair : targetMappingInDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingInSize--;
				if (setContainNodeID(targetMappingOut, tempPair.first)) {
					targetMappingBoth.erase(tempPair.first);
					--targetBothInOutSize;
				}
				targetMappingIn.erase(tempPair.first);
				targetMappingInDepth[tempPair.first] = 0;

			}
		}


		for (const auto& tempPair : targetMappingOutDepth) {
			if (tempPair.second == searchDepth) {
				targetMappingOutSize--;
				if (setContainNodeID(targetMappingIn, tempPair.first))
				{
					targetMappingBoth.erase(tempPair.first);
					--targetBothInOutSize;
				}
				targetMappingOut.erase(tempPair.first);
				targetMappingOutDepth[tempPair.first] = 0;
				//	deleteList.push_back(tempPair.first);
			}
		}
		if (targetMappingInDepth[targetNodeID] < searchDepth && targetMappingInDepth[targetNodeID]) {
			targetMappingInSize++;
			if (setContainNodeID(targetMappingOut, targetNodeID)) {
				targetMappingBoth.insert(targetNodeID);
				++targetBothInOutSize;
			}
			targetMappingIn.insert(targetNodeID);
		}
		if (targetMappingOutDepth[targetNodeID] < searchDepth && targetMappingOutDepth[targetNodeID])
		{
			targetMappingOutSize++;
			if (setContainNodeID(targetMappingIn, targetNodeID)) {
				targetMappingBoth.insert(targetNodeID);
				++targetBothInOutSize;
			}
			targetMappingOut.insert(targetNodeID);
		}
		searchDepth--;

		return;

	}
	bool isCoverQueryGraph()const {
		if (queryGraph.graphSize() == searchDepth)	return true;
		return false;
	};
	MapType getMap(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}

};




