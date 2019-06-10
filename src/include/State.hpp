#pragma once
#include<vector>
#include<time.h>
#include"Graph.hpp"
#include"Pair.hpp"
#include"NodeSet.hpp"
#include<assert.h>
#include<map>
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<limits>
#include"common.h"
#include<si_marcos.h>


#define NO_MAP SIZE_MAX
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
	typedef vector<NodeIDType> MapType;
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

	//	typedef unordered_set<NodeIDType> NodeSetType;
	typedef NodeSet NodeSetType;

private:

	MapType mapping, mappingAux; //from query to target
	const GraphType& targetGraph, &queryGraph;
	NodeSetType targetUnmap, targetIn, targetOut, targetBoth,
		queryUnmap, queryIn, queryOut, queryBoth;
	int targetInSize = 0, targetOutSize = 0, queryInSize = 0,
		queryOutSize = 0, targetBothSize = 0, queryBothSize = 0;

	/*	unordered_map<NodeIDType, int> targetMappingInDepth, targetMappingOutDepth,
			queryMappingInDepth, queryMappingOutDepth;*/
	vector<int> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;
	vector<int> targetMappingInRefTimes, targetMappingOutRefTimes,
		queryMappingInRefTimes, queryMappingOutRefTimes;
	size_t searchDepth;
	vector<int> targetInRefTimesCla, targetOutRefTimesCla, targetInBothRefTimesCla, targetOutBothRefTimesCla,
		queryInRefTimesCla, queryOutRefTimesCla, queryInBothRefTimesCla, queryOutBothRefTimesCla;
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

	template<typename _Key, typename _Value>
	bool mapIsCovered(const unordered_map<_Key, _Value> & querym, unordered_map<_Key, _Value> & targetm)const {
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


		vector<size_t> queryInDepth, queryOutDepth, queryInBothDepth, queryOutBothDepth,
			targetInDepth, targetOutDepth, targetInBothDepth, targetOutBothDepth;

		queryInDepth.resize(searchDepth + 1);
		queryOutDepth.resize(searchDepth + 1);
		queryInBothDepth.resize(searchDepth + 1);
		queryOutBothDepth.resize(searchDepth + 1);

		targetInDepth.resize(searchDepth + 1);
		targetOutDepth.resize(searchDepth + 1);
		targetInBothDepth.resize(searchDepth + 1);
		targetOutBothDepth.resize(searchDepth + 1);

		size_t queryInCount = 0, targetInCount = 0, queryNotTCount = 0, targetNotTCount = 0,
			queryOutCount = 0, targetOutCount = 0, queryBothCount = 0, targetBothCount = 0;

		for (const auto& tempEdge : querySourceNode.getOutEdges()) {
			const auto& queryTargetNodeID = tempEdge.getTargetNodeID();
			//this tempnode have been mapped
			if (NOT_IN_NODESET(queryUnmap, queryTargetNodeID)) {
				const auto& targetTargetNodeID = mapping[queryTargetNodeID];
				const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);
				if (targetSourceNode.existSameTypeEdgeToNode(targetTargetNode, tempEdge) == false) return false;
			}
			else {

				const bool o = IN_NODESET(queryOut, queryTargetNodeID);
				const bool i = IN_NODESET(queryIn, queryTargetNodeID);
				const bool b = (o && i);
				size_t inDepth, outDepth;
				if (o) outDepth = queryMappingOutDepth[queryTargetNodeID];
				if (i)inDepth = queryMappingInDepth[queryTargetNodeID];
				if (o && !b) {
					queryOutDepth[outDepth]++;
					++queryOutCount;
				}
				if (i && !b) {
					queryInDepth[inDepth]++;
					++queryInCount;
				}
				if (b) {

					queryOutBothDepth[outDepth]++;
					queryInBothDepth[inDepth]++;
					++queryBothCount;
				}
				if (!b && !i && !o) ++queryNotTCount;
			}
		}

		for (const auto& tempEdge : targetSourceNode.getOutEdges()) {
			const auto& targetTargetNodeID = tempEdge.getTargetNodeID();

			if (NOT_IN_NODESET(targetUnmap, targetTargetNodeID)) {
				if (induceGraph == false)continue;
				const auto & queryTargetNodeID = mappingAux[targetTargetNodeID];
				const auto & queryTargetNode = queryGraph.getNode(queryTargetNodeID);
				if (querySourceNode.existSameTypeEdgeToNode(queryTargetNode, tempEdge) == false) return false;
			}
			else {

				const bool o = IN_NODESET(targetOut, targetTargetNodeID);
				const bool i = IN_NODESET(targetIn, targetTargetNodeID);
				const bool b = (i && o);
				size_t inDepth, outDepth;
				if (o) outDepth = targetMappingOutDepth[targetTargetNodeID];
				if (i) inDepth = targetMappingInDepth[targetTargetNodeID];
				if (o && !b) {
					targetOutDepth[outDepth]++;
					++targetOutCount;
				}
				if (i && !b) {
					targetInDepth[inDepth]++;
					++targetInCount;
				}
				if (b) {

					targetOutBothDepth[outDepth]++;
					targetInBothDepth[inDepth]++;
					++targetBothCount;
				}

				if (!b && !i && !o) ++targetNotTCount;
			}
		}

		if (queryNotTCount > targetNotTCount) return false;
		if (queryInCount > targetInCount || queryOutCount > targetOutCount || queryBothCount > targetBothCount)return false;

		for (auto i = 1; i <= searchDepth; ++i) {
			if (queryInBothDepth[i] > targetInBothDepth[i] || queryOutBothDepth[i] > targetOutBothDepth[i] ||
				queryInDepth[i] > targetInDepth[i] || queryOutDepth[i] > targetOutDepth[i])return false;
		}

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

		vector<size_t> queryInDepth, queryOutDepth, queryInBothDepth, queryOutBothDepth,
			targetInDepth, targetOutDepth, targetInBothDepth, targetOutBothDepth;

		queryInDepth.resize(searchDepth + 1);
		queryOutDepth.resize(searchDepth + 1);
		queryInBothDepth.resize(searchDepth + 1);
		queryOutBothDepth.resize(searchDepth + 1);

		targetInDepth.resize(searchDepth + 1);
		targetOutDepth.resize(searchDepth + 1);
		targetInBothDepth.resize(searchDepth + 1);
		targetOutBothDepth.resize(searchDepth + 1);

		for (const auto& tempEdge : queryTargetNode.getInEdges()) {
			const auto& querySourceNodeID = tempEdge.getSourceNodeID();
			if (NOT_IN_NODESET(queryUnmap, querySourceNodeID)) {

				const auto& targetSourceNodeID = mapping[querySourceNodeID];
				const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);
				if (targetTargetNode.existSameTypeEdgeFromNode(targetSourceNode, tempEdge) == false) return false;
			}
			else {

				const bool o = IN_NODESET(queryOut, querySourceNodeID);
				const bool i = IN_NODESET(queryIn, querySourceNodeID);
				const bool b = (o && i);
				size_t inDepth, outDepth;
				if (o) outDepth = queryMappingOutDepth[querySourceNodeID];
				if (i) inDepth = queryMappingInDepth[querySourceNodeID];
				if (o && !b) {
					queryOutDepth[outDepth]++;
					++queryOutCount;
				}
				if (i && !b) {
					queryInDepth[inDepth]++;
					++queryInCount;
				}
				if (b) {
					queryOutBothDepth[outDepth]++;
					queryInBothDepth[inDepth]++;

					++queryBothCount;

				}
				if (!b && !i && !o) ++queryNotTCount;
			}
		}

		for (const auto& tempEdge : targetTargetNode.getInEdges()) {
			const auto& targetSourceNodeID = tempEdge.getSourceNodeID();

			if (NOT_IN_NODESET(targetUnmap, targetSourceNodeID)) {
				if (induceGraph == false)continue;
				const auto & querySourceNodeID = mappingAux[targetSourceNodeID];
				const auto & querySourceNode = queryGraph.getNode(querySourceNodeID);
				if (queryTargetNode.existSameTypeEdgeFromNode(querySourceNode, tempEdge) == false) return false;
			}
			else {

				const bool o = IN_NODESET(targetOut, targetSourceNodeID);
				const bool i = IN_NODESET(targetIn, targetSourceNodeID);
				const bool b = (o && i);
				size_t inDepth, outDepth;
				if (o) outDepth = targetMappingOutDepth[targetSourceNodeID];
				if (i) inDepth = targetMappingInDepth[targetSourceNodeID];
				if (o && !b) {
					targetOutDepth[outDepth]++;
					++targetOutCount;
				}
				if (i && !b) {
					targetInDepth[inDepth]++;
					++targetInCount;
				}
				if (b) {
					targetOutBothDepth[outDepth]++;
					targetInBothDepth[inDepth]++;
					++targetBothCount;
				}
				if (!b && !i && !o) ++targetNotTCount;
			}
		}

		if (queryNotTCount > targetNotTCount) return false;
		if (queryInCount > targetInCount || queryOutCount > targetOutCount || queryBothCount > targetBothCount)return false;

		for (auto i = 1; i <= searchDepth; ++i) {
			if (queryInBothDepth[i] > targetInBothDepth[i] || queryOutBothDepth[i] > targetOutBothDepth[i] ||
				queryInDepth[i] > targetInDepth[i] || queryOutDepth[i] > targetOutDepth[i])return false;
		}

		return true;
	}

	bool inOutRefRule()const {
		for (auto i = 1; i < queryInRefTimesCla.size(); ++i) {
			if (queryInBothRefTimesCla[i] > targetInBothRefTimesCla[i])return false;
			int querySub = queryInRefTimesCla[i] - queryInBothRefTimesCla[i];
			int targetSub = targetInRefTimesCla[i] - targetInBothRefTimesCla[i];
			if (querySub > targetSub)return false;
		}

		for (auto i = 1; i < queryOutRefTimesCla.size(); ++i) {
			if (queryOutBothRefTimesCla[i] > targetOutBothRefTimesCla[i])return false;
			int querySub = queryOutRefTimesCla[i] - queryOutBothRefTimesCla[i];
			int targetSub = targetOutRefTimesCla[i] - targetOutBothRefTimesCla[i];
			if (querySub > targetSub)return false;
		}

		return true;

	}

	void seleteMatchOrder() {
		//	NodeSetType nodeNotInMatchSet = queryUnmap, inSet, outSet, bothSet;
		auto nodeNotInMatchSet = queryUnmap.getSet();
		typedef decltype(nodeNotInMatchSet) Set;
		Set inSet, outSet, bothSet;
		inSet.reserve(queryGraph.size() << 1);
		outSet.reserve(queryGraph.size() << 1);
		bothSet.reserve(queryGraph.size() << 1);

		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		vector<int>  inMap, outMap;

		inMap.resize(queryGraph.size() + 1);
		outMap.resize(queryGraph.size() + 1);
		const auto calNodeMatchPoint = [&](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + inMap[node.getID()] + outMap[node.getID()];
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set & s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {
				const auto & tempNode = queryGraph.getNode(tempNodeID);
				const auto tempPoint = calNodeMatchPoint(tempNode);
				if (tempPoint > nodePoint) {
					answer = &tempNode;
					nodePoint = tempPoint;
				}
			}
			return NodeMatchPointPair(answer, nodePoint);
		};


		matchSequence.reserve(queryGraph.size() + 5);

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
			const auto & node = *nodePointPair.getKey();

			const auto & nodeID = node.getID();
			matchSequence.push_back(nodeID);
			nodeNotInMatchSet.erase(nodeID);
			inSet.erase(nodeID);
			outSet.erase(nodeID);
			bothSet.erase(nodeID);

			for (const auto& tempEdge : node.getInEdges()) {
				const auto sourceNodeID = tempEdge.getSourceNodeID();
				if (IN_SET(nodeNotInMatchSet, sourceNodeID)) {
					inSet.insert(sourceNodeID);
					inMap[sourceNodeID]++;
					if (IN_SET(outSet, sourceNodeID)) bothSet.insert(sourceNodeID);
				}
			}
			for (const auto& tempEdge : node.getOutEdges()) {
				const auto targetNodeID = tempEdge.getTargetNodeID();
				if (IN_SET(nodeNotInMatchSet, targetNodeID)) {
					outSet.insert(targetNodeID);
					outMap[targetNodeID]++;
					if (IN_SET(inSet, targetNodeID))bothSet.insert(targetNodeID);
				}
			}

		}
	}

public:
	StateVF2(const GraphType & _t, const GraphType & _q, bool _induceGraph) :targetGraph(_t), queryGraph(_q), induceGraph(_induceGraph) {

		const auto queryGraphSize = queryGraph.size();
		const auto targetGraphSize = targetGraph.size();
		const auto queryHashSize = calHashSuitableSize(queryGraphSize);
		const auto targetHashSize = calHashSuitableSize(targetGraphSize);
		mapping.resize(queryGraphSize);
		mappingAux.resize(targetGraphSize);
		for (auto &i : mapping) i = NO_MAP;
		for (auto &i : mappingAux) i = NO_MAP;
		targetIn = NodeSet(targetGraphSize);
		targetOut = NodeSet(targetGraphSize);
		targetBoth = NodeSet(targetGraphSize);
		targetUnmap = NodeSet(targetGraphSize);

		queryIn = NodeSet(queryGraphSize);
		queryOut = NodeSet(queryGraphSize);
		queryBoth = NodeSet(queryGraphSize);
		queryUnmap = NodeSet(queryGraphSize);

		targetMappingInDepth.resize(targetGraphSize);
		targetMappingOutDepth.resize(targetGraphSize);
		queryMappingInDepth.resize(queryGraphSize);
		queryMappingOutDepth.resize(queryGraphSize);


		targetMappingInRefTimes.resize(targetGraphSize);
		targetMappingOutRefTimes.resize(targetGraphSize);
		queryMappingInRefTimes.resize(queryGraphSize);
		queryMappingOutRefTimes.resize(queryGraphSize);


		searchDepth = 0;
		size_t targetInMax = 0, targetOutMax = 0, queryInMax = 0, queryOutMax = 0;
		for (auto& tempNode : targetGraph.getAllNodes())
		{
			targetInMax = max(targetInMax, tempNode.getInEdgesNum());
			targetOutMax = max(targetOutMax, tempNode.getOutEdgesNum());
			targetUnmap.insert(getNodeID(tempNode));
		}
		for (auto& tempNode : queryGraph.getAllNodes())
		{
			queryInMax = max(queryInMax, tempNode.getInEdgesNum());
			queryOutMax = max(queryOutMax, tempNode.getOutEdgesNum());
			queryUnmap.insert(getNodeID(tempNode));
		}
		//ofc not a subgraph of target Graph.
		if (targetInMax < queryInMax || targetOutMax < queryOutMax) stillConsistentAfterAdd = false;
		swap(targetInMax, targetOutMax);
		swap(queryInMax, queryOutMax);
		targetInRefTimesCla = vector<int>(targetInMax + 1, 0);
		targetOutRefTimesCla = vector<int>(targetOutMax + 1, 0);
		targetInBothRefTimesCla = vector<int>(targetInMax + 1, 0);
		targetOutBothRefTimesCla = vector<int>(targetOutMax + 1, 0);
		targetInRefTimesCla[0] = targetGraphSize;
		targetOutRefTimesCla[0] = targetGraphSize;
		queryInRefTimesCla = vector<int>(queryInMax + 1, 0);
		queryOutRefTimesCla = vector<int>(queryOutMax + 1, 0);
		queryInBothRefTimesCla = vector<int>(queryInMax + 1, 0);
		queryOutBothRefTimesCla = vector<int>(queryOutMax + 1, 0);
		queryInRefTimesCla[0] = queryGraphSize;
		queryOutRefTimesCla[0] = queryGraphSize;

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
		const bool queryNodeInIn = IN_NODESET(queryIn, queryNodeToMatchID);
		const bool queryNodeInOut = IN_NODESET(queryOut, queryNodeToMatchID);
		const NodeSetType * tempNodeSetPointer;
		if (queryNodeInIn && queryNodeInOut) tempNodeSetPointer = &targetBoth;
		else if (queryNodeInIn) tempNodeSetPointer = &targetIn;
		else if (queryNodeInOut)tempNodeSetPointer = &targetOut;
		else tempNodeSetPointer = &targetUnmap;



		const auto queryNodeInRefTimes = queryMappingInRefTimes[queryNodeToMatchID];
		const auto queryNodeOutRefTimes = queryMappingOutRefTimes[queryNodeToMatchID];
		const auto queryNodeInDepth = queryMappingInDepth[queryNodeToMatchID];
		const auto queryNodeOutDepth = queryMappingOutDepth[queryNodeToMatchID];
		answer.reserve(targetInSize + targetOutSize);
		const auto & targetNodeToMatchSet = *tempNodeSetPointer;
		//	for (const auto& targetNodeToMatchID : targetNodeToMatchSet) {
		TRAVERSE_NODESET(targetNodeToMatchID, targetNodeToMatchSet)
		{
			if (twoNodesMayMatch(queryNodeToMatchID, targetNodeToMatchID) == false)continue;

			// it will be ditched because of sourceRule in next depth .
			const auto targetNodeInRefTimes = targetMappingInRefTimes[targetNodeToMatchID];
			if (induceGraph) {
				if (queryNodeInRefTimes != targetNodeInRefTimes) continue;
			}
			else if (queryNodeInRefTimes > targetNodeInRefTimes) continue;
			// it will be ditched because of targetRule in next depth .
			const auto targetNodeOutRefTimes = targetMappingOutRefTimes[targetNodeToMatchID];
			if (induceGraph) {
				if (queryNodeOutRefTimes != targetNodeOutRefTimes) continue;
			}
			else if (queryNodeOutRefTimes > targetNodeOutRefTimes) continue;

			const auto targetNodeInDepth = targetMappingInDepth[targetNodeToMatchID];
			if (induceGraph) { if (queryNodeInDepth != targetNodeInDepth)continue; }
			else if (queryNodeInDepth < targetNodeInDepth)continue;
			const auto targetNodeOutDepth = targetMappingOutDepth[targetNodeToMatchID];
			if (induceGraph) { if (queryNodeOutDepth != targetNodeOutDepth)continue; }
			else if (queryNodeOutDepth < targetNodeOutDepth)continue;

			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));

		}
		return answer;

	}
	bool checkCanditatePairIsAddable(const MapPair & cp)const
	{

		//	seeMappingContent();
		//	seePairID(cp);
		const bool answer = sourceRule(cp) && targetRule(cp);
		//	cout << answer << endl;


		return answer;

	}
	void addCanditatePairToMapping(const MapPair & cp)
	{
		mapping[cp.getKey()] = cp.getValue();
		mappingAux[cp.getValue()] = cp.getKey();
		const auto targetNodeID = cp.getValue();
		const auto queryNodeID = cp.getKey();

		targetUnmap.erase(targetNodeID);
		queryUnmap.erase(queryNodeID);



		bool inInSet = IN_NODESET(targetIn, targetNodeID);
		bool inOutSet = IN_NODESET(targetOut, targetNodeID);
		auto inRefTimes = targetMappingInRefTimes[targetNodeID];
		auto outRefTimes = targetMappingOutRefTimes[targetNodeID];
		if (inInSet) {
			--targetInSize;
			targetIn.erase(targetNodeID);
			targetInRefTimesCla[inRefTimes]--;
		}
		if (inOutSet) {
			--targetOutSize;
			targetOut.erase(targetNodeID);
			targetOutRefTimesCla[inRefTimes]--;
		}
		if (inInSet && inOutSet) {
			targetBoth.erase(targetNodeID);
			targetInBothRefTimesCla[inRefTimes]--;
			targetOutBothRefTimesCla[outRefTimes]--;
			--targetBothSize;
		}



		inInSet = IN_NODESET(queryIn, queryNodeID);
		inOutSet = IN_NODESET(queryOut, queryNodeID);
		inRefTimes = queryMappingInRefTimes[queryNodeID];
		outRefTimes = queryMappingOutRefTimes[queryNodeID];
		if (inInSet) {
			--queryInSize;
			queryIn.erase(queryNodeID);
			queryInRefTimesCla[inRefTimes]--;
		}
		if (inOutSet) {
			--queryOutSize;
			queryOut.erase(queryNodeID);
			queryOutRefTimesCla[inRefTimes]--;
		}
		if (inInSet && inOutSet) {
			queryBoth.erase(queryNodeID);
			queryInBothRefTimesCla[inRefTimes]--;
			queryOutBothRefTimesCla[outRefTimes]--;
			--queryBothSize;
		}



		const auto& targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		const auto addOneIfExist = [](const NodeSetType & set, NodeSetType & both, const NodeIDType nodeID, int & size) {
			if (IN_NODESET(set, nodeID)) {
				++size;
				both.insert(nodeID);
				return true;
			}
			return false;
		};

		const size_t targetMappingInSizeOld = targetInSize, targetMappingOutSizeOld = targetOutSize, targetBothInOutSizeOld = targetBothSize,
			queryMappingInSizeOld = queryInSize, queryMappingOutSizeOld = queryOutSize, queryBothInOutSizeOld = queryBothSize;


		// A  :  inSet  new     outSet  no
		// B  :  inSet  new     outSet  old
		// C  :  inSet  new     outSet  new
		// D  :  inSet  old     outSet  new
		// E  :  inSet  no      outSet  new
		// so	inSet add = A+B+C
		//		outSet add = C+D+E
		//		BothSet add = B+C+D


		size_t queryA = 0, queryB = 0, queryC = 0, queryD = 0, queryE = 0,
			targetA = 0, targetB = 0, targetC = 0, targetD = 0, targetE = 0;

		// there is a variables' name using error about [In/Out]RefTimesCla
		// like below  getInEdges should use targetOutRefTimesCla to record info
		// but for coding convenience , I use targetInRefTimesCla 
		// so you can see the swap function in the construct function .
		for (const auto& tempEdge : targetNodePointer->getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			// was not be mapped
			const bool n = IN_NODESET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODESET(targetOut, nodeID);
			const bool i = IN_NODESET(targetIn, nodeID);
			const bool b = (o && i);

			if (!i) {
				targetIn.insert(nodeID);
				targetMappingInDepth[nodeID] = searchDepth;
				++targetInSize;
				addOneIfExist(targetOut, targetBoth, nodeID, targetBothSize);
			}

			auto &refTimes = targetMappingInRefTimes[nodeID];
			targetInRefTimesCla[refTimes]--;
			++refTimes;
			targetInRefTimesCla[refTimes]++;

			if (o) {
				//this node was not in inSet before add action
				if (!i) {
					const auto outRefTimes = targetMappingOutRefTimes[nodeID];

					targetOutBothRefTimesCla[outRefTimes]++;
					++targetB;
				}
				if (i)targetInBothRefTimesCla[refTimes - 1]--;
				targetInBothRefTimesCla[refTimes]++;
			}

		}
		for (const auto& tempEdge : targetNodePointer->getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_NODESET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODESET(targetOut, nodeID);
			const bool i = IN_NODESET(targetIn, nodeID);
			const bool b = (o && i);
			bool temp = false;
			if (!o) {
				targetOut.insert(nodeID);
				targetMappingOutDepth[nodeID] = searchDepth;
				++targetOutSize;
				temp = addOneIfExist(targetIn, targetBoth, nodeID, targetBothSize);
			}

			auto &refTimes = targetMappingOutRefTimes[nodeID];
			targetOutRefTimesCla[refTimes]--;
			++refTimes;
			targetOutRefTimesCla[refTimes]++;

			if (i) {
				if (!o) {
					const auto inRefTimes = targetMappingInRefTimes[nodeID];

					targetInBothRefTimesCla[inRefTimes]++;
					if (targetMappingInDepth[nodeID] == searchDepth) ++targetC;
				}
				if (o)targetOutBothRefTimesCla[refTimes - 1]--;
				targetOutBothRefTimesCla[refTimes]++;
			}
			if (temp == false && !o)++targetE;
		}


		for (const auto& tempEdge : queryNodePointer->getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			//		queryMappingInRefTimes[sourceNodeID]++;
			const bool n = IN_NODESET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODESET(queryOut, nodeID);
			const bool i = IN_NODESET(queryIn, nodeID);
			const bool b = (o && i);

			if (!i) {
				queryIn.insert(nodeID);
				queryMappingInDepth[nodeID] = searchDepth;
				++queryInSize;
				addOneIfExist(queryOut, queryBoth, nodeID, queryBothSize);
			}

			auto &refTimes = queryMappingInRefTimes[nodeID];
			queryInRefTimesCla[refTimes]--;
			++refTimes;
			queryInRefTimesCla[refTimes]++;

			if (o) {
				//this node was not in inSet before add action
				if (!i) {
					const auto outRefTimes = queryMappingOutRefTimes[nodeID];
					queryOutBothRefTimesCla[outRefTimes]++;
					++queryB;
				}
				if (i)queryInBothRefTimesCla[refTimes - 1]--;
				queryInBothRefTimesCla[refTimes]++;
			}
		}
		for (const auto& tempEdge : queryNodePointer->getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_NODESET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODESET(queryOut, nodeID);
			const bool i = IN_NODESET(queryIn, nodeID);
			const bool b = (o && i);
			bool temp = false;
			if (!o) {
				queryOut.insert(nodeID);
				queryMappingOutDepth[nodeID] = searchDepth;
				++queryOutSize;
				temp = addOneIfExist(queryIn, queryBoth, nodeID, queryBothSize);
			}

			auto &refTimes = queryMappingOutRefTimes[nodeID];
			queryOutRefTimesCla[refTimes]--;
			++refTimes;
			queryOutRefTimesCla[refTimes]++;

			if (i) {
				if (!o) {
					const auto inRefTimes = queryMappingInRefTimes[nodeID];

					queryInBothRefTimesCla[inRefTimes]++;
					if (queryMappingInDepth[nodeID] == searchDepth) ++queryC;
				}
				if (o)queryOutBothRefTimesCla[refTimes - 1]--;
				queryOutBothRefTimesCla[refTimes]++;
			}
			if (temp == false && !o)++queryE;

		}

		const size_t targetInAdd = targetInSize - targetMappingInSizeOld,
			targetOutAdd = targetOutSize - targetMappingOutSizeOld,
			targetBothAdd = targetBothSize - targetBothInOutSizeOld,
			queryInAdd = queryInSize - queryMappingInSizeOld,
			queryOutAdd = queryOutSize - queryMappingOutSizeOld,
			queryBothAdd = queryBothSize - queryBothInOutSizeOld;

		queryA = queryInAdd - queryB - queryC;
		targetA = targetInAdd - targetB - targetC;
		queryD = queryOutAdd - queryC - queryE;
		targetD = targetOutAdd - targetC - targetE;
		//	if (induceGraph && (queryInAdd > targetInAdd || queryOutAdd > targetOutAdd  || queryBothAdd > targetBothAdd))stillConsistentAfterAdd = false;
		if (induceGraph && (queryA > targetA || queryE > targetE || queryC > targetC))stillConsistentAfterAdd = false;
		//	if (induceGraph && (queryA > targetA || queryE > targetE || queryC > targetC || queryB > targetB || queryD > targetD))stillConsistentAfterAdd = false;
		else stillConsistentAfterAdd = true;

		return;
	}
	void deleteCanditatePairToMapping(const MapPair & cp)
	{
		if (cp.getKey() == 48 && cp.getValue() == 24) {
			int a = 0;
		}
		mapping[cp.getKey()] = NO_MAP;
		mappingAux[cp.getValue()] = NO_MAP;
		const auto& queryNodeID = cp.getKey();
		const auto& targetNodeID = cp.getValue();
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);

		targetUnmap.insert(targetNodeID);
		queryUnmap.insert(queryNodeID);


		for (const auto& tempEdge : queryNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();

			const bool n = IN_NODESET(queryUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_NODESET(queryBoth, nodeID);

			auto &refTimes = queryMappingInRefTimes[nodeID];
			auto &nodeDepth = queryMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				--queryInSize;
				queryIn.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto &outRefTimes = queryMappingOutRefTimes[nodeID];
					queryOutBothRefTimesCla[outRefTimes]--;
					queryInBothRefTimesCla[refTimes]--;
					queryBoth.erase(nodeID);
					--queryBothSize;
				}
				nodeDepth = 0;
			}
			else if (b) {
				queryInBothRefTimesCla[refTimes]--;
				queryInBothRefTimesCla[refTimes - 1]++;
			}
			queryInRefTimesCla[refTimes]--;
			queryInRefTimesCla[refTimes - 1]++;

			refTimes--;

		}
		for (const auto& tempEdge : queryNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();

			const bool n = IN_NODESET(queryUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_NODESET(queryBoth, nodeID);

			auto &refTimes = queryMappingOutRefTimes[nodeID];
			auto &nodeDepth = queryMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				--queryOutSize;
				queryOut.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto inRefTimes = queryMappingInRefTimes[nodeID];
					queryOutBothRefTimesCla[refTimes]--;
					queryInBothRefTimesCla[inRefTimes]--;
					queryBoth.erase(nodeID);
					--queryBothSize;
				}
				nodeDepth = 0;
			}
			else if (b) {
				queryOutBothRefTimesCla[refTimes]--;
				queryOutBothRefTimesCla[refTimes - 1]++;
			}

			queryOutRefTimesCla[refTimes]--;
			queryOutRefTimesCla[refTimes - 1]++;

			refTimes--;
		}
		for (const auto& tempEdge : targetNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			const bool n = IN_NODESET(targetUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_NODESET(targetBoth, nodeID);

			auto &refTimes = targetMappingInRefTimes[nodeID];
			auto &nodeDepth = targetMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				--targetInSize;
				targetIn.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto outRefTimes = targetMappingOutRefTimes[nodeID];
					targetOutBothRefTimesCla[outRefTimes]--;
					targetInBothRefTimesCla[refTimes]--;
					targetBoth.erase(nodeID);
					--targetBothSize;
				}
				nodeDepth = 0;
			}
			else if (b) {
				targetInBothRefTimesCla[refTimes]--;
				targetInBothRefTimesCla[refTimes - 1]++;
			}

			targetInRefTimesCla[refTimes]--;
			targetInRefTimesCla[refTimes - 1]++;

			refTimes--;
		}
		for (const auto& tempEdge : targetNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_NODESET(targetUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_NODESET(targetBoth, nodeID);

			auto &refTimes = targetMappingOutRefTimes[nodeID];
			auto &nodeDepth = targetMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				--targetOutSize;
				targetOut.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto inRefTimes = targetMappingInRefTimes[nodeID];
					targetOutBothRefTimesCla[refTimes]--;
					targetInBothRefTimesCla[inRefTimes]--;
					targetBoth.erase(nodeID);
					--targetBothSize;
				}
				nodeDepth = 0;
			}
			else if (b) {
				targetOutBothRefTimesCla[refTimes]--;
				targetOutBothRefTimesCla[refTimes - 1]++;
			}
			targetOutRefTimesCla[refTimes]--;
			targetOutRefTimesCla[refTimes - 1]++;

			refTimes--;


		}
		if (queryMappingInDepth[queryNodeID]) {
			++queryInSize;
			queryIn.insert(queryNodeID);

			const auto refTimes = queryMappingInRefTimes[queryNodeID];
			queryInRefTimesCla[refTimes]++;
			if (IN_NODESET(queryOut, queryNodeID)) {

				queryInBothRefTimesCla[refTimes]++;
				const auto outRefTimes = queryMappingOutRefTimes[queryNodeID];
				queryOutBothRefTimesCla[outRefTimes]++;

				queryBoth.insert(queryNodeID);
				++queryBothSize;
			}

		}
		if (queryMappingOutDepth[queryNodeID])
		{
			++queryOutSize;
			queryOut.insert(queryNodeID);

			const auto refTimes = queryMappingOutRefTimes[queryNodeID];
			queryOutRefTimesCla[refTimes]++;
			if (IN_NODESET(queryIn, queryNodeID)) {
				queryOutBothRefTimesCla[refTimes]++;
				const auto inRefTimes = queryMappingInRefTimes[queryNodeID];
				queryInBothRefTimesCla[inRefTimes]++;
				queryBoth.insert(queryNodeID);
				++queryBothSize;
			}

		}
		if (targetMappingInDepth[targetNodeID]) {
			targetInSize++;
			targetIn.insert(targetNodeID);

			const auto refTimes = targetMappingInRefTimes[targetNodeID];
			targetInRefTimesCla[refTimes]++;
			if (IN_NODESET(targetOut, targetNodeID)) {

				targetInBothRefTimesCla[refTimes]++;
				const auto outRefTimes = targetMappingOutRefTimes[targetNodeID];
				targetOutBothRefTimesCla[outRefTimes]++;

				targetBoth.insert(targetNodeID);
				++targetBothSize;
			}

		}
		if (targetMappingOutDepth[targetNodeID])
		{
			targetOutSize++;
			targetOut.insert(targetNodeID);

			const auto refTimes = targetMappingOutRefTimes[targetNodeID];
			targetOutRefTimesCla[refTimes]++;

			if (IN_NODESET(targetIn, targetNodeID)) {
				targetOutBothRefTimesCla[refTimes]++;
				const auto inRefTimes = targetMappingInRefTimes[targetNodeID];
				targetInBothRefTimesCla[inRefTimes]++;
				targetBoth.insert(targetNodeID);
				++targetBothSize;
			}

		}
		if (targetOutBothRefTimesCla[1] > 1E5) {
			int a = 0;
		}
		searchDepth--;

		return;

	}
	bool isCoverQueryGraph()const {
		if (queryGraph.size() == searchDepth)	return true;
		return false;
	};
	MapType getMap(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}

};




