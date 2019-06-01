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


template<typename _GraphType>
class StateVF2 : public State<_GraphType> {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;

	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef State<GraphType> StateBase;
	typedef const NodeType* NodeCPointer;
	typedef typename StateBase::MapPair MapPair;
	typedef typename StateBase::MapType MapType;

	typedef unordered_set<NodeIDType> NodeSetType;

private:

	MapType mapping, mappingAux; //from query to target
	const GraphType& targetGraph, &queryGraph;
	NodeSetType targetGraphUnmap, targetMappingIn, targetMappingOut, targetMappingBoth,
		queryGraphUnmap, queryMappingIn, queryMappingOut, queryMappingBoth;
	size_t targetMappingInSize = 0, targetMappingOutSize = 0, queryMappingInSize = 0,
		queryMappingOutSize = 0, targetBothInOutSize = 0, queryBothInOutSize = 0;

	unordered_map<NodeIDType, size_t> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;

	unordered_map<NodeIDType, size_t> targetMappingInRefTimes, targetMappingOutRefTimes,
		queryMappingInRefTimes, queryMappingOutRefTimes;
	size_t searchDepth;

	vector<NodeIDType> matchSequence;
	bool induceGraph = true;

	inline void seePairID(const MapPair& cp)const {
		cout << "Ready To Mapping" << "(" << cp.getKey() << "," << cp.getValue() << ")" << endl;
	}
	inline void seeMappingContent()const {
		for (const auto it : mapping) {
			if (it.second) {
				const auto queryNodeID = it.first;
				const auto targetNodeID = it.second;
				cout << '(' << queryNodeID << "," << targetNodeID << ')' << endl;

			}
		}
	}

	inline bool setContainNodeID(const NodeSetType& s, const NodeIDType& nodeID) const {
		return (s.find(nodeID) != s.end());
	}
	inline bool setNotContainNodeID(const NodeSetType & s, const NodeIDType & nodeID)const {
		return (s.find(nodeID) == s.end());
	}
	bool stillConsistentAfterAdd = true;
	//because the match order is immutable , search depth decides which node to be matched;
	inline NodeIDType selectNodeToCalCanditates()const
	{
		return matchSequence[searchDepth];
	}
	//same label and target node edges' number should cover query node's.
	inline bool twoNodesMayMatch(NodeIDType queryNodeID, NodeIDType targetNodeID)const
	{
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);
		return ((queryNode.isSameType(targetNode)) && (queryNode <= targetNode));

	}


	NodeIDType getMapNodeID(const MapType &m, const NodeIDType &id)const {
		const auto pair = m.find(id);
		assert((pair != m.end()) && "mapping not exist the map about this node");
		return pair->second;
	}
	size_t getNodeDepth(const unordered_map<NodeIDType, size_t> &m, const NodeIDType &nodeID)const {
		const auto &tempPair = m.find(nodeID);
		if (tempPair == m.end()) return size_t(0);
		else return tempPair->second;
	}
	template<typename _Key, typename _Value>
	bool mapIsCovered(const unordered_map<_Key, _Value> &querym, unordered_map<_Key, _Value> &targetm)const {
		if (induceGraph) {
			for (const auto pair : querym) {
				if (targetm[pair.first] < pair.second) return false;
			}
			return true;
		}
		else {
			return true;
		}
	};

	//check the mapping is still consistent after add this pair
	bool sourceRule(const MapPair & cp) const
	{
		const auto& querySourceNodeID = cp.getKey();
		const auto& targetSourceNodeID = cp.getValue();

		const auto& querySourceNode = queryGraph.getNode(querySourceNodeID);
		const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);

		unordered_map<size_t, size_t> queryTargetInDepth, queryTargetOutDepth, queryTargetBothDepth,
			targetTargetInDepth, targetTargetOutDepth, targetTargetBothDepth;

		queryTargetInDepth.reserve(queryMappingInSize);
		queryTargetOutDepth.reserve(queryMappingOutSize);
		queryTargetBothDepth.reserve(queryBothInOutSize);
		targetTargetInDepth.reserve(targetMappingInSize);
		targetTargetOutDepth.reserve(targetMappingOutSize);
		targetTargetBothDepth.reserve(targetBothInOutSize);

		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0,
			queryOutCount = 0, targetOutCount = 0, queryBothCount = 0, targetBothCount = 0;

		for (const auto& tempEdge : querySourceNode.getOutEdges()) {
			const auto& queryTargetNodeID = tempEdge.getTargetNodeID();
			//this tempnode have been mapped
			if (setNotContainNodeID(queryGraphUnmap, queryTargetNodeID)) {
				const auto & targetTargetNodeID = getMapNodeID(mapping, queryTargetNodeID);
				const auto & targetTargetNode = targetGraph.getNode(targetTargetNodeID);
				if (targetSourceNode.existSameTypeEdgeToNode(targetTargetNode, tempEdge) == false) return false;
			}
			else {
				const auto queryTargetNodeDepth = getNodeDepth(queryMappingOutDepth, queryTargetNodeID);
				const bool o = setContainNodeID(queryMappingOut, queryTargetNodeID);
				const bool i = setContainNodeID(queryMappingIn, queryTargetNodeID);
				const bool b = (o&&i);
				if (o && !b) {
					queryTargetOutDepth[queryTargetNodeDepth]++;
					++queryOutCount;
				}
				if (i && !b) {
					queryTargetInDepth[queryTargetNodeDepth]++;
					++queryInCount;
				}
				if (b) {
					queryTargetBothDepth[queryTargetNodeDepth]++;
					++queryBothCount;
				}
				if (!b && !i && !o) ++queryNotTCount;
			}
		}


		for (const auto& tempEdge : targetSourceNode.getOutEdges()) {
			const auto& targetTargetNodeID = tempEdge.getTargetNodeID();

			if (setNotContainNodeID(targetGraphUnmap, targetTargetNodeID)) {
				if (induceGraph == false)continue;
				const auto & queryTargetNodeID = getMapNodeID(mappingAux, targetTargetNodeID);
				const auto & queryTargetNode = queryGraph.getNode(queryTargetNodeID);
				if (querySourceNode.existSameTypeEdgeToNode(queryTargetNode, tempEdge) == false) return false;
			}
			else {
				const auto targetTargetNodeDepth = getNodeDepth(targetMappingOutDepth, targetTargetNodeID);
				const bool o = setContainNodeID(targetMappingOut, targetTargetNodeID);
				const bool i = setContainNodeID(targetMappingIn, targetTargetNodeID);

				const bool b = (i&&o);
				if (o && !b) {
					targetTargetOutDepth[targetTargetNodeDepth]++;
					++targetOutCount;
				}
				if (i && !b) {
					targetTargetInDepth[targetTargetNodeDepth]++;
					++targetInCount;
				}
				if (b) {
					targetTargetBothDepth[targetTargetNodeDepth]++;
					++targetBothCount;
				}

				if (!b && !i && !o) ++targetNotTCount;
			}
		}

		if (queryNotTCount > targetNotTCount) return false;

		if (mapIsCovered(queryTargetInDepth, targetTargetInDepth) == false || mapIsCovered(queryTargetOutDepth, targetTargetOutDepth) == false
			|| mapIsCovered(queryTargetBothDepth, targetTargetBothDepth) == false)return false;
		return true;

	}
	bool targetRule(const MapPair & cp)const
	{
		const auto& queryTargetNodeID = cp.getKey();
		const auto& targetTargetNodeID = cp.getValue();

		const auto& queryTargetNode = queryGraph.getNode(queryTargetNodeID);
		const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);

		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0,
			queryOutCount = 0, targetOutCount = 0, queryBothCount = 0, targetBothCount = 0;

		unordered_map<size_t, size_t> querySourceInDepth, querySourceOutDepth, querySourceBothDepth,
			targetSourceInDepth, targetSourceOutDepth, targetSourceBothDepth;

		querySourceInDepth.reserve(queryMappingInSize);
		querySourceOutDepth.reserve(queryMappingOutSize);
		querySourceBothDepth.reserve(queryBothInOutSize);
		targetSourceInDepth.reserve(targetMappingInSize);
		targetSourceOutDepth.reserve(targetMappingOutSize);
		targetSourceBothDepth.reserve(targetBothInOutSize);

		for (const auto& tempEdge : queryTargetNode.getInEdges()) {
			const auto& querySourceNodeID = tempEdge.getSourceNodeID();
			if (setNotContainNodeID(queryGraphUnmap, querySourceNodeID)) {
				const auto & targetSourceNodeID = getMapNodeID(mapping, querySourceNodeID);
				const auto & targetSourceNode = targetGraph.getNode(targetSourceNodeID);
				if (targetTargetNode.existSameTypeEdgeFromNode(targetSourceNode, tempEdge) == false) return false;
			}
			else {
				const auto querySourceNodeDepth = getNodeDepth(queryMappingInDepth, querySourceNodeID);
				const bool o = setContainNodeID(queryMappingOut, querySourceNodeID);
				const bool i = setContainNodeID(queryMappingIn, querySourceNodeID);
				const bool b = (o && i);

				if (o && !b) {
					querySourceOutDepth[querySourceNodeDepth]++;
				}
				if (i && !b) {
					querySourceInDepth[querySourceNodeDepth]++;
				}
				if (b) {
					querySourceBothDepth[querySourceNodeDepth]++;
				}
				if (!b && !i && !o) ++queryNotTCount;
			}
		}

		for (const auto& tempEdge : targetTargetNode.getInEdges()) {
			const auto& targetSourceNodeID = tempEdge.getSourceNodeID();

			if (setNotContainNodeID(targetGraphUnmap, targetSourceNodeID)) {
				if (induceGraph == false)continue;
				const auto & querySourceNodeID = getMapNodeID(mappingAux, targetSourceNodeID);
				const auto & querySourceNode = queryGraph.getNode(querySourceNodeID);
				if (queryTargetNode.existSameTypeEdgeFromNode(querySourceNode, tempEdge) == false) return false;
			}
			else {
				const auto targetSourceNodeDepth = getNodeDepth(targetMappingInDepth, targetSourceNodeID);
				const bool o = setContainNodeID(targetMappingOut, targetSourceNodeID);
				const bool i = setContainNodeID(targetMappingIn, targetSourceNodeID);
				const bool b = (o && i);

				if (o && !b) {
					targetSourceOutDepth[targetSourceNodeDepth]++;

				}
				if (i && !b) {
					targetSourceInDepth[targetSourceNodeDepth]++;

				}
				if (b) {
					targetSourceBothDepth[targetSourceNodeDepth]++;
				}
				if (!b && !i && !o) ++targetNotTCount;
			}
		}

		if (queryNotTCount > targetNotTCount) return false;
		if (mapIsCovered(querySourceInDepth, targetSourceInDepth) == false || mapIsCovered(querySourceOutDepth, targetSourceOutDepth) == false
			|| mapIsCovered(querySourceBothDepth, targetSourceBothDepth) == false)return false;

		return true;
	}


	bool inOutRefRule()const {
		//there is a similar cut rule for non-induce subgraph , but it is not on the schedule for now .
		if (!induceGraph)return true;

		//ref times  -- node number

		unordered_map<size_t, size_t>  queryInRef, queryOutRef, targetInRef, targetOutRef,
			queryBothInRef, targetBothInRef, queryBothOutRef, targetBothOutRef;

		const auto getRefTimes = [](const unordered_map<NodeIDType, size_t> &m, const NodeIDType &nodeID) {
			const auto &tempPair = m.find(nodeID);
			assert(tempPair != m.end() && " this node in in/out/both Set but ref times is zero ?");
			return tempPair->second;
		};
		const auto cmpRefMap = [](const unordered_map<size_t, size_t> &querym, unordered_map<size_t, size_t> &targetm) {
			for (const auto pair : querym) {
				if (targetm[pair.first] < pair.second) return false;
			}
			return true;
		};

		queryOutRef.reserve(queryMappingOutSize << 1);
		targetOutRef.reserve(targetMappingOutSize << 1);
		queryBothOutRef.reserve(queryBothInOutSize << 1);
		targetBothOutRef.reserve(targetBothInOutSize << 1);
		for (const auto &nodeID : queryMappingOut) {
			const auto refTimes = getRefTimes(queryMappingOutRefTimes, nodeID);
			if (setContainNodeID(queryMappingBoth, nodeID))	queryBothOutRef[refTimes]++;
			else queryOutRef[refTimes]++;

		}

		for (const auto &nodeID : targetMappingOut) {
			const auto refTimes = getRefTimes(targetMappingOutRefTimes, nodeID);
			if (setContainNodeID(targetMappingBoth, nodeID)) targetBothOutRef[refTimes]++;
			else targetOutRef[refTimes]++;
		}
		if (cmpRefMap(queryOutRef, targetOutRef) == false || cmpRefMap(queryBothOutRef, targetBothOutRef) == false)return false;


		queryInRef.reserve(queryMappingInSize << 1);
		targetInRef.reserve(targetMappingInSize << 1);
		queryBothInRef.reserve(queryBothInOutSize << 1);
		targetBothInRef.reserve(targetBothInOutSize << 1);

		for (const auto &nodeID : queryMappingIn) {
			const auto refTimes = getRefTimes(queryMappingInRefTimes, nodeID);
			if (setContainNodeID(queryMappingBoth, nodeID))				queryBothInRef[refTimes]++;
			else	queryInRef[refTimes]++;

		}

		for (const auto &nodeID : targetMappingIn) {
			const auto refTimes = getRefTimes(targetMappingInRefTimes, nodeID);
			if (setContainNodeID(targetMappingBoth, nodeID))targetBothInRef[refTimes]++;

			else targetInRef[refTimes]++;

		}
		if (cmpRefMap(queryInRef, targetInRef) == false || cmpRefMap(queryBothInRef, targetBothInRef) == false)return false;

		return true;
	}

#define RULE_3
#ifdef RULE_1
	void seleteMatchOrder() {
		NodeSetType nodeNotInMatchSet = queryGraphUnmap;
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		unordered_map<NodeIDType, char> map;
		const auto calNodeMatchPoint = [](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum();
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&]() {
			//		const auto seleteAGoodNodeToMatch = [](const GraphType &queryGraph, NodeSetType &nodeNotInMatchSet) {
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
			order.push(nodePointPair);
			while (order.empty() == false) {
				//			for (auto &i : matchSequence)cout << i << " "; cout << endl;
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
				if (nodeNotInMatchSet.empty()) {
					while (order.empty() == false) {
						matchSequence.push_back(order.top().getKey()->getID());
						order.pop();
					}
				}
			}

		}


	}
#elif defined(RULE_3)
	void seleteMatchOrder() {
		NodeSetType nodeNotInMatchSet = queryGraphUnmap, inSet, outSet, bothSet;
		inSet.reserve(queryGraph.graphSize() << 1);
		outSet.reserve(queryGraph.graphSize() << 1);
		bothSet.reserve(queryGraph.graphSize() << 1);

		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		unordered_map<NodeIDType, char>  inMap, outMap;
		inMap.reserve(queryGraph.graphSize() << 1);
		outMap.reserve(queryGraph.graphSize() << 1);
		const auto calNodeMatchPoint = [&](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + inMap[node.getID()] + outMap[node.getID()];
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const NodeSetType &s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			const bool inAndOutSetNull = (inSet.empty() && outSet.empty());
			for (const auto& tempNodeID : s) {
				if (inAndOutSetNull == false && (setNotContainNodeID(inSet, tempNodeID) && setNotContainNodeID(outSet, tempNodeID))) continue;
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




		while (nodeNotInMatchSet.empty() == false) {
			NodeMatchPointPair nodePointPair;
			if (bothSet.empty() == false) nodePointPair = seleteAGoodNodeToMatch(bothSet);
			else if (inSet.empty() == false || outSet.empty() == false) {
				const auto inPair = seleteAGoodNodeToMatch(inSet);
				const auto outPair = seleteAGoodNodeToMatch(outSet);
				nodePointPair = (inPair.getValue() > outPair.getValue()) ? inPair : outPair;
			}
			else nodePointPair = seleteAGoodNodeToMatch(nodeNotInMatchSet);
			assert(nodePointPair.getKey() != nullptr && "error happened");
			const auto &node = *nodePointPair.getKey();

			const auto &nodeID = node.getID();
			matchSequence.push_back(nodeID);
			nodeNotInMatchSet.erase(nodeID);
			inSet.erase(nodeID);
			outSet.erase(nodeID);
			bothSet.erase(nodeID);

			for (const auto& tempEdge : node.getInEdges()) {
				const auto sourceNodeID = tempEdge.getSourceNodeID();
				if (setContainNodeID(nodeNotInMatchSet, sourceNodeID)) {
					inSet.insert(sourceNodeID);
					inMap[sourceNodeID]++;
					if (setContainNodeID(outSet, sourceNodeID)) bothSet.insert(sourceNodeID);
				}
			}
			for (const auto& tempEdge : node.getOutEdges()) {
				const auto targetNodeID = tempEdge.getTargetNodeID();
				if (setContainNodeID(nodeNotInMatchSet, targetNodeID)) {
					outSet.insert(targetNodeID);
					outMap[targetNodeID]++;
					if (setContainNodeID(inSet, targetNodeID))bothSet.insert(targetNodeID);
				}
			}

		}
	}

#endif



public:
	StateVF2(const GraphType & _t, const GraphType & _q, bool _induceGraph) :targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {

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


		targetMappingInRefTimes.reserve(targetHashSize);
		targetMappingOutRefTimes.reserve(targetHashSize);
		queryMappingInRefTimes.reserve(queryHashSize);
		queryMappingOutRefTimes.reserve(queryHashSize);


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
		if (stillConsistentAfterAdd == false) return answer;
		if (inOutRefRule() == false)return answer;

		const auto & queryNodeToMatchID = selectNodeToCalCanditates();
		const bool queryNodeInIn = setContainNodeID(queryMappingIn, queryNodeToMatchID);
		const bool queryNodeInOut = setContainNodeID(queryMappingOut, queryNodeToMatchID);
		const NodeSetType * tempNodeSetPointer;
		if (queryNodeInIn && queryNodeInOut) tempNodeSetPointer = &targetMappingBoth;
		else if (queryNodeInIn) tempNodeSetPointer = &targetMappingIn;
		else if (queryNodeInOut)tempNodeSetPointer = &targetMappingOut;
		else tempNodeSetPointer = &targetGraphUnmap;

		const auto getRefTimes = [](const unordered_map<NodeIDType, size_t> &m, const NodeIDType &nodeID) {
			const auto &tempPair = m.find(nodeID);
			if (tempPair == m.end()) return size_t(0);
			else return tempPair->second;
		};
		const auto getNodeDepth = [](const unordered_map<NodeIDType, size_t> &m, const NodeIDType &nodeID) {
			const auto &tempPair = m.find(nodeID);
			if (tempPair == m.end()) return size_t(0);
			else return tempPair->second;
		};

		const auto queryNodeInRefTimes = getRefTimes(queryMappingInRefTimes, queryNodeToMatchID);
		const auto queryNodeOutRefTimes = getRefTimes(queryMappingOutRefTimes, queryNodeToMatchID);
		const auto queryNodeInDepth = getNodeDepth(queryMappingInDepth, queryNodeToMatchID);
		const auto queryNodeOutDepth = getNodeDepth(queryMappingOutDepth, queryNodeToMatchID);
		answer.reserve(targetMappingInSize + targetMappingOutSize);
		const auto & targetNodeToMatchSet = *tempNodeSetPointer;
		for (const auto& targetNodeToMatchID : targetNodeToMatchSet) {
			if (twoNodesMayMatch(queryNodeToMatchID, targetNodeToMatchID) == false)continue;

			// it will be ditched because of sourceRule in next depth .
			const auto targetNodeInRefTimes = getRefTimes(targetMappingInRefTimes, targetNodeToMatchID);
			if (induceGraph) {
				if (queryNodeInRefTimes != targetNodeInRefTimes) continue;
			}
			else if (queryNodeInRefTimes > targetNodeInRefTimes) continue;
			// it will be ditched because of sourceRule in next depth .
			const auto targetNodeOutRefTimes = getRefTimes(targetMappingOutRefTimes, targetNodeToMatchID);
			if (induceGraph) {
				if (queryNodeOutRefTimes != targetNodeOutRefTimes) continue;
			}
			else if (queryNodeOutRefTimes > targetNodeOutRefTimes) continue;

			const auto targetNodeInDepth = getNodeDepth(targetMappingInDepth, targetNodeToMatchID);
			if (induceGraph) { if (queryNodeInDepth != targetNodeInDepth)continue; }
			else if (queryNodeInDepth < targetNodeInDepth)continue;
			const auto targetNodeOutDepth = getNodeDepth(targetMappingOutDepth, targetNodeToMatchID);
			if (induceGraph) { if (queryNodeOutDepth != targetNodeOutDepth)continue; }
			else if (queryNodeOutDepth < targetNodeOutDepth)continue;
			
			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));

		}
		return answer;

	}
	bool checkCanditatePairIsAddable(const MapPair & cp)const
	{

		const bool answer = sourceRule(cp) && targetRule(cp);
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


		const auto calSetSizeAfterErase = [](NodeSetType & inSet, NodeSetType & outSet, NodeSetType & bothSet, const NodeIDType & nodeID,
			size_t & inSize, size_t & outSize, size_t & bothSize)
		{
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

		calSetSizeAfterErase(targetMappingIn, targetMappingOut, targetMappingBoth,
			targetNodeID, targetMappingInSize, targetMappingOutSize, targetBothInOutSize);
		calSetSizeAfterErase(queryMappingIn, queryMappingOut, queryMappingBoth,
			queryNodeID, queryMappingInSize, queryMappingOutSize, queryBothInOutSize);


		const auto& targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		const auto addOneIfExist = [](const NodeSetType & set, NodeSetType & both, const NodeIDType nodeID, size_t & size) {
			if (set.find(nodeID) != set.end()) {
				++size;
				both.insert(nodeID);
				return true;
			}
			return false;
		};
		size_t targetInAdd = 0, targetOutAdd = 0, targetBothAdd = 0,
			queryInAdd = 0, queryOutAdd = 0, queryBothAdd = 0;
		unordered_set<NodeIDType> targetInAddSet;
		targetInAddSet.reserve(targetNodePointer->getInEdgesNum() << 1);

		for (const auto& tempEdge : targetNodePointer->getInEdges()) {
			const auto& sourceNodeID = tempEdge.getSourceNodeID();
			targetMappingInRefTimes[sourceNodeID]++;
			// was not be mapped
			if (setContainNodeID(targetGraphUnmap, sourceNodeID)) {

				if (targetMappingInDepth[sourceNodeID] == 0) {
					targetMappingIn.insert(sourceNodeID);
					targetMappingInDepth[sourceNodeID] = searchDepth;
					++targetMappingInSize;
					++targetInAdd;
					targetInAddSet.insert(sourceNodeID);
					if (addOneIfExist(targetMappingOut, targetMappingBoth, sourceNodeID, targetBothInOutSize) == true) {
						targetBothAdd++;
						--targetInAdd;
					}
				}

			}
		}
		for (const auto& tempEdge : targetNodePointer->getOutEdges()) {
			const auto& targetNodeID = tempEdge.getTargetNodeID();
			targetMappingOutRefTimes[targetNodeID]++;
			if (setContainNodeID(targetGraphUnmap, targetNodeID)) {


				if (targetMappingOutDepth[targetNodeID] == 0) {
					targetMappingOut.insert(targetNodeID);
					targetMappingOutDepth[targetNodeID] = searchDepth;
					++targetMappingOutSize;
					++targetOutAdd;
					if (addOneIfExist(targetMappingIn, targetMappingBoth, targetNodeID, targetBothInOutSize)) {
						targetBothAdd++;
						if (setContainNodeID(targetInAddSet, targetNodeID))--targetInAdd;
					};
				}
			}
		}
		unordered_set<NodeIDType> queryInAddSet;
		queryInAddSet.reserve(queryNodePointer->getInEdgesNum() << 1);
		
		for (const auto& tempEdge : queryNodePointer->getInEdges()) {
			const auto& sourceNodeID = tempEdge.getSourceNodeID();
			queryMappingInRefTimes[sourceNodeID]++;
			if (setContainNodeID(queryGraphUnmap, sourceNodeID)) {


				if (queryMappingInDepth[sourceNodeID] == 0) {
					queryMappingIn.insert(sourceNodeID);
					queryMappingInDepth[sourceNodeID] = searchDepth;
					++queryMappingInSize;
					++queryInAdd;
					queryInAddSet.insert(sourceNodeID);
					if (addOneIfExist(queryMappingOut, queryMappingBoth, sourceNodeID, queryBothInOutSize)) {
						++queryBothAdd;
						--queryInAdd;
					};
				}
			}
		}
		for (const auto& tempEdge : queryNodePointer->getOutEdges()) {
			const auto& targetNodeID = tempEdge.getTargetNodeID();
			queryMappingOutRefTimes[targetNodeID]++;
			if (setContainNodeID(queryGraphUnmap, targetNodeID)) {

				if (queryMappingOutDepth[targetNodeID] == 0) {
					queryMappingOut.insert(targetNodeID);
					queryMappingOutDepth[targetNodeID] = searchDepth;
					++queryMappingOutSize;
					++queryOutAdd;
					if (addOneIfExist(queryMappingIn, queryMappingBoth, targetNodeID, queryBothInOutSize)) {
						++queryBothAdd;
						if (setContainNodeID(queryInAddSet, targetNodeID)) --queryInAdd;
					};
				}
			}
		}
	
		if (induceGraph && (queryInAdd > targetInAdd || queryOutAdd > targetOutAdd || queryBothAdd > targetBothAdd))stillConsistentAfterAdd = false;
		else stillConsistentAfterAdd = true;

		return;
	}
	void deleteCanditatePairToMapping(const MapPair & cp)
	{

		mapping.erase(cp.getKey());
		mappingAux.erase(cp.getValue());
		const auto& queryNodeID = cp.getKey();
		const auto& targetNodeID = cp.getValue();
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);

		targetGraphUnmap.insert(targetNodeID);
		queryGraphUnmap.insert(queryNodeID);

		for (const auto &tempEdge : queryNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			queryMappingInRefTimes[nodeID]--;
		}
		for (const auto &tempEdge : queryNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			queryMappingOutRefTimes[nodeID]--;
		}
		for (const auto &tempEdge : targetNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			targetMappingInRefTimes[nodeID]--;
		}
		for (const auto &tempEdge : targetNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			targetMappingOutRefTimes[nodeID]--;
		}

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

		if (queryMappingInDepth[queryNodeID]) {
			++queryMappingInSize;
			if (setContainNodeID(queryMappingOut, queryNodeID)) {
				queryMappingBoth.insert(queryNodeID);
				++queryBothInOutSize;
			}
			queryMappingIn.insert(queryNodeID);
		}
		if (queryMappingOutDepth[queryNodeID])
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

			}
		}
		if (targetMappingInDepth[targetNodeID]) {
			targetMappingInSize++;
			if (setContainNodeID(targetMappingOut, targetNodeID)) {
				targetMappingBoth.insert(targetNodeID);
				++targetBothInOutSize;
			}
			targetMappingIn.insert(targetNodeID);
		}
		if (targetMappingOutDepth[targetNodeID])
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




