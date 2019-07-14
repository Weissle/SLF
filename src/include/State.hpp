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
#include<cstring>
#define INDUCE_ISO
//#define NORMAL_ISO
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

	virtual vector<MapPair> calCandidatePairs(const NodeIDType id)const = 0;
	virtual bool checkCanditatePairIsAddable(const MapPair& cp) = 0;
	virtual void addCanditatePairToMapping(const MapPair& cp) = 0;
	virtual void deleteCanditatePairToMapping(const MapPair& cp) = 0;
	virtual bool isCoverQueryGraph()const = 0;
	virtual MapType getMap(bool showNotCoverWarning = true) const = 0;
};
#ifdef INDUCE_ISO
template<typename T>
void new_two_dim_array(T**& p, size_t one, size_t two = -1) {
	if (two == -1)two = one;
	p = new T * [one];
	for (auto i = 0; i < one; ++i) p[i] = new T[two]();
	return;
}
template<typename T>
void delete_two_dim_array(T**& p, size_t one) {
	for (auto i = 0; i < one; ++i) delete[](p[i]);
	delete[]p;
	return;
}
template<typename T>
void clear_two_dim_array(T**& p, size_t one, size_t two = -1) {
	if (two == -1)two = one;
	for (auto i = 0; i < one; ++i) memset(p[i], 0, sizeof(int) * two);
	return;
}
#endif
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
	const GraphType& targetGraph, & queryGraph;
	size_t searchDepth = 0;
	MapType mapping;
	MapType mappingAux; //from query to target

	NodeSetType targetUnmap, targetIn, targetOut, targetBoth,
		queryUnmap, queryIn, queryOut, queryBoth;

	vector<int> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;
	vector<int> targetMappingInRefTimes, targetMappingOutRefTimes,
		queryMappingInRefTimes, queryMappingOutRefTimes;
#ifdef INDUCE_ISO
	int** DRin, ** DRout;
#endif
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



	template<typename _Key, typename _Value>
	bool mapIsCovered(const unordered_map<_Key, _Value>& querym, unordered_map<_Key, _Value>& targetm)const {
#ifdef INDUCE_ISO
		for (const auto pair : querym) {
			if (targetm[pair.first] < pair.second) return false;
		}
		return true;

#elif defined(NORMAL_ISO)
		return true;

#endif
	};


	//check the mapping is still consistent after add this pair
	bool sourceRule(const MapPair& cp)
	{
		const auto& querySourceNodeID = cp.getKey();
		const auto& targetSourceNodeID = cp.getValue();

		const auto& querySourceNode = queryGraph.getNode(querySourceNodeID);
		const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);


#ifdef INDUCE_ISO
		typedef FSPair<size_t, size_t> DRPair;
		unordered_map< FSPair< DRPair, DRPair>, size_t>  bothIO;
		bothIO.reserve(calHashSuitableSize(targetSourceNode.getOutEdgesNum()));
		clear_two_dim_array(DRin, searchDepth + 1);
		clear_two_dim_array(DRout, searchDepth + 1);

#elif defined(NORMAL_ISO)
		size_t inCount = 0, outCount = 0, bothCount = 0;
#endif
		size_t notTCount = 0;


		for (const auto& tempEdge : targetSourceNode.getOutEdges()) {
			const auto& targetTargetNodeID = tempEdge.getTargetNodeID();

			if (NOT_IN_SET(targetUnmap, targetTargetNodeID)) {
#ifdef INDUCE_ISO
				const auto queryTargetNodeID = mappingAux[targetTargetNodeID];
				const auto& queryTargetNode = queryGraph.getNode(queryTargetNodeID);
				if (querySourceNode.existSameTypeEdgeToNode(queryTargetNode, tempEdge) == false) return false;
#endif
			}
			else if (targetTargetNodeID == targetSourceNodeID) {
#ifdef INDUCE_ISO
				if (querySourceNode.existSameTypeEdgeToNode(querySourceNode, tempEdge) == false)return false;
#endif
			}
			else {

				const bool o = IN_SET(targetOut, targetTargetNodeID);
				const bool i = IN_SET(targetIn, targetTargetNodeID);
				const bool b = (i && o);

#ifdef INDUCE_ISO
				size_t inDepth, outDepth, inRef, outRef;
				if (o) {
					outDepth = targetMappingOutDepth[targetTargetNodeID];
					outRef = targetMappingOutRefTimes[targetTargetNodeID];
				}
				if (i) {
					inDepth = targetMappingInDepth[targetTargetNodeID];
					inRef = targetMappingInRefTimes[targetTargetNodeID];
				}
				if (o && !b) {
					DRout[outDepth][outRef]++;
				}
				if (i && !b) {
					DRin[inDepth][inRef]++;

				}
				if (b) {
					bothIO[FSPair<DRPair, DRPair>(DRPair(inDepth, inRef), DRPair(outDepth, outRef))]++;
				}
#elif defined(NORMAL_ISO)
				if (o && !b) {
					++outCount;
				}
				if (i && !b) {
					++inCount;
				}
				if (b) {
					++bothCount;
				}
#endif
				if (!i && !o) ++notTCount;
			}
		}


		for (const auto& tempEdge : querySourceNode.getOutEdges()) {
			const auto& queryTargetNodeID = tempEdge.getTargetNodeID();
			//this tempnode have been mapped
			if (NOT_IN_SET(queryUnmap, queryTargetNodeID)) {
				const auto targetTargetNodeID = mapping[queryTargetNodeID];
				const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);
				if (targetSourceNode.existSameTypeEdgeToNode(targetTargetNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetSourceNode.existSameTypeEdgeToNode(targetSourceNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_SET(queryOut, queryTargetNodeID);
				const bool i = IN_SET(queryIn, queryTargetNodeID);
				const bool b = (o && i);
#ifdef INDUCE_ISO
				size_t inDepth, outDepth, inRef, outRef;
				if (o) {
					outDepth = queryMappingOutDepth[queryTargetNodeID];
					outRef = queryMappingOutRefTimes[queryTargetNodeID];
				}
				if (i) {
					inDepth = queryMappingInDepth[queryTargetNodeID];
					inRef = queryMappingInRefTimes[queryTargetNodeID];
				}

				if (o && !b) {
					if (DRout[outDepth][outRef] == 0)return false;
					else --DRout[outDepth][outRef];


				}
				if (i && !b) {
					if (DRin[inDepth][inRef] == 0) return false;
					else --DRin[inDepth][inRef];

				}
				if (b) {
					auto& temp = bothIO[FSPair<DRPair, DRPair>(DRPair(inDepth, inRef), DRPair(outDepth, outRef))];
					if (temp)--temp;
					else return false;
				}
#elif defined(NORMAL_ISO)
				if (o && !b) {
					if (outCount)--outCount;
					else return false;
				}
				if (i && !b) {
					if (inCount) --inCount;
					else return false;

				}
				if (b) {
					if (bothCount)--bothCount;
					else return false;
				}
#endif
				if (!i && !o) {
					if (notTCount)--notTCount;
					else return false;
				}
			}
		}

		return true;

	}

	bool targetRule(const MapPair& cp)
	{
		const auto& queryTargetNodeID = cp.getKey();
		const auto& targetTargetNodeID = cp.getValue();

		const auto& queryTargetNode = queryGraph.getNode(queryTargetNodeID);
		const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);

#ifdef INDUCE_ISO
		typedef FSPair<size_t, size_t> DRPair;
		unordered_map< FSPair< DRPair, DRPair>, size_t>  bothIO;
		bothIO.reserve(calHashSuitableSize(targetTargetNode.getInEdgesNum()));
		clear_two_dim_array(DRin, searchDepth + 1);
		clear_two_dim_array(DRout, searchDepth + 1);

#elif defined(NORMAL_ISO)
		size_t inCount = 0, outCount = 0, bothCount = 0;
#endif
		size_t notTCount = 0;


		for (const auto& tempEdge : targetTargetNode.getInEdges()) {
			const auto& targetSourceNodeID = tempEdge.getSourceNodeID();

			if (NOT_IN_SET(targetUnmap, targetSourceNodeID)) {
#ifdef INDUCE_ISO
				const auto& querySourceNodeID = mappingAux[targetSourceNodeID];
				const auto& querySourceNode = queryGraph.getNode(querySourceNodeID);
				if (queryTargetNode.existSameTypeEdgeFromNode(querySourceNode, tempEdge) == false) return false;
#endif
			}
			else if (targetTargetNodeID == targetSourceNodeID) {
#ifdef INDUCE_ISO
				if (queryTargetNode.existSameTypeEdgeFromNode(queryTargetNode, tempEdge) == false)return false;
#endif
			}
			else {

				const bool o = IN_SET(targetOut, targetSourceNodeID);
				const bool i = IN_SET(targetIn, targetSourceNodeID);
				const bool b = (o && i);
#ifdef INDUCE_ISO
				size_t inDepth, outDepth, inRef, outRef;
				if (o) {
					outDepth = targetMappingOutDepth[targetSourceNodeID];
					outRef = targetMappingOutRefTimes[targetSourceNodeID];
				}
				if (i) {
					inRef = targetMappingInRefTimes[targetSourceNodeID];
					inDepth = targetMappingInDepth[targetSourceNodeID];
				}
				if (o && !b) {
					DRout[outDepth][outRef]++;

				}
				if (i && !b) {
					DRin[inDepth][inRef]++;

				}
				if (b) {
					bothIO[FSPair<DRPair, DRPair>(DRPair(inDepth, inRef), DRPair(outDepth, outRef))]++;
				}
#elif defined(NORMAL_ISO)
				if (o && !b) {
					++outCount;
				}
				if (i && !b) {
					++inCount;
				}
				if (b) {
					++bothCount;
				}
#endif
				if (!i && !o) ++notTCount;
			}
		}


		for (const auto& tempEdge : queryTargetNode.getInEdges()) {
			const auto& querySourceNodeID = tempEdge.getSourceNodeID();
			if (NOT_IN_SET(queryUnmap, querySourceNodeID)) {

				const auto& targetSourceNodeID = mapping[querySourceNodeID];
				const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);
				if (targetTargetNode.existSameTypeEdgeFromNode(targetSourceNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetTargetNode.existSameTypeEdgeFromNode(targetTargetNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_SET(queryOut, querySourceNodeID);
				const bool i = IN_SET(queryIn, querySourceNodeID);
				const bool b = (o && i);
#ifdef INDUCE_ISO
				size_t inDepth, outDepth, inRef, outRef;
				if (o) {

					outDepth = queryMappingOutDepth[querySourceNodeID];
					outRef = queryMappingOutRefTimes[querySourceNodeID];
				}
				if (i) {
					inRef = queryMappingInRefTimes[querySourceNodeID];
					inDepth = queryMappingInDepth[querySourceNodeID];
				}
				if (o && !b) {
					if (DRout[outDepth][outRef] == 0)return false;
					else --DRout[outDepth][outRef];
				}
				if (i && !b) {
					if (DRin[inDepth][inRef] == 0) return false;
					else --DRin[inDepth][inRef];


				}
				if (b) {
					auto& temp = bothIO[FSPair<DRPair, DRPair>(DRPair(inDepth, inRef), DRPair(outDepth, outRef))];
					if (temp)--temp;
					else return false;
				}
#elif defined(NORMAL_ISO)
				if (o && !b) {
					if (outCount)--outCount;
					else return false;
				}
				if (i && !b) {
					if (inCount) --inCount;
					else return false;

				}
				if (b) {
					if (bothCount)--bothCount;
					else return false;
				}
#endif
				if (!i && !o) {
					if (notTCount)--notTCount;
					else return false;
				}
			}
		}



		return true;
	}

public:
	StateVF2(const GraphType& _t, const GraphType& _q) :queryGraph(_q), targetGraph(_t) {

		const auto queryGraphSize = queryGraph.size();
		const auto targetGraphSize = targetGraph.size();
		const auto queryHashSize = calHashSuitableSize(queryGraphSize);
		const auto targetHashSize = calHashSuitableSize(targetGraphSize);

		mappingAux.resize(targetGraphSize);
		for (auto& i : mappingAux) i = NO_MAP;
        mapping.resize(queryGraphSize);
        for (auto& i : mapping) i = NO_MAP;

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

		for (const auto& node : queryGraph.nodes()) queryUnmap.insert(node.id());
		for (const auto& node : targetGraph.nodes()) targetUnmap.insert(node.id());
#ifdef INDUCE_ISO
		new_two_dim_array(DRin, queryGraphSize + 1);
		new_two_dim_array(DRout, queryGraphSize + 1);

#elif defined(NORMAL_ISO)
#endif

	};
	StateVF2() = default;
	~StateVF2() {
#ifdef INDUCE_ISO
		const auto queryGraphSize = queryGraph.size();
		delete_two_dim_array(DRin, queryGraphSize + 1);
		delete_two_dim_array(DRout, queryGraphSize + 1);
#elif defined(NORMAL_ISO)

#endif

	}

public:
	//public function
	vector<MapPair> calCandidatePairs(const NodeIDType id)const
	{
		vector<MapPair> answer;
		if (stillConsistentAfterAdd == false) {
			return answer;
		}

		const auto& queryNodeToMatchID = id;
		const bool queryNodeInIn = IN_SET(queryIn, queryNodeToMatchID);
		const bool queryNodeInOut = IN_SET(queryOut, queryNodeToMatchID);
		const NodeSetType* tempNodeSetPointer;
		if (queryNodeInIn && queryNodeInOut) tempNodeSetPointer = &targetBoth;
		else if (queryNodeInIn) tempNodeSetPointer = &targetIn;
		else if (queryNodeInOut)tempNodeSetPointer = &targetOut;
		else tempNodeSetPointer = &targetUnmap;

		const auto& queryNode = queryGraph.getNode(queryNodeToMatchID);
		const auto queryNodeInRefTimes = queryMappingInRefTimes[queryNodeToMatchID];
		const auto queryNodeOutRefTimes = queryMappingOutRefTimes[queryNodeToMatchID];
		const auto queryNodeInDepth = queryMappingInDepth[queryNodeToMatchID];
		const auto queryNodeOutDepth = queryMappingOutDepth[queryNodeToMatchID];
		answer.reserve(max(targetIn.size(), targetOut.size()));
		const auto& targetNodeToMatchSet = *tempNodeSetPointer;



		TRAVERSE_SET(targetNodeToMatchID, targetNodeToMatchSet)
		{
			const auto& targetNode = targetGraph.getNode(targetNodeToMatchID);
			if (queryNode.isSameType(targetNode) == false || queryNode >= targetNode) continue;

#ifdef INDUCE_ISO
			// it will be ditched because of sourceRule in next depth .
			if (queryNodeInRefTimes != targetMappingInRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeOutRefTimes != targetMappingOutRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeInDepth != targetMappingInDepth[targetNodeToMatchID])continue;
			if (queryNodeOutDepth != targetMappingOutDepth[targetNodeToMatchID])continue;
#elif defined(NORMAL_ISO)
			if (queryNodeInRefTimes > targetMappingInRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeOutRefTimes > targetMappingOutRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeInDepth < targetMappingInDepth[targetNodeToMatchID])continue;
			if (queryNodeOutDepth < targetMappingOutDepth[targetNodeToMatchID])continue;
#endif

			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));

		}
		return answer;

	}
	bool checkCanditatePairIsAddable(const MapPair& cp)
	{
		//	seeMappingContent();
		//	seePairID(cp);
		const bool answer = sourceRule(cp) && targetRule(cp);
		//	cout << answer << endl;
		return answer;

	}
	void addCanditatePairToMapping(const MapPair& cp)
	{
		const auto targetNodeID = cp.getValue();
		const auto queryNodeID = cp.getKey();
		mapping[queryNodeID] = targetNodeID;
		mappingAux[targetNodeID] = queryNodeID;


		targetUnmap.erase(targetNodeID);
		queryUnmap.erase(queryNodeID);

		targetIn.erase(targetNodeID);
		targetOut.erase(targetNodeID);
		targetBoth.erase(targetNodeID);
		queryIn.erase(queryNodeID);
		queryOut.erase(queryNodeID);
		queryBoth.erase(queryNodeID);



		const auto& targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

#ifdef INDUCE_ISO
		const size_t targetMappingInSizeOld = targetIn.size(), targetMappingOutSizeOld = targetOut.size(), targetBothInOutSizeOld = targetBoth.size(),
			queryMappingInSizeOld = queryIn.size(), queryMappingOutSizeOld = queryOut.size(), queryBothInOutSizeOld = queryBoth.size();


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
#elif defined(NORMAL_ISO)

#endif
		// there is a variables' name using error about [In/Out]RefTimesCla
		// like below  getInEdges should use targetOutRefTimesCla to record info
		// but for coding convenience , I use targetInRefTimesCla 
		// so you can see the swap function in the construct function .
		for (const auto& tempEdge : targetNodePointer->getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			// was not be mapped
			const bool n = IN_SET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_SET(targetOut, nodeID);
			const bool i = IN_SET(targetIn, nodeID);
			const bool b = (o && i);

			if (!i) {
				targetIn.insert(nodeID);
				targetMappingInDepth[nodeID] = searchDepth;
				if (o)targetBoth.insert(nodeID);
			}

			++targetMappingInRefTimes[nodeID];
#ifdef INDUCE_ISO
			if (o) {
				//this node was not in inSet before add action
				if (!i) {

					++targetB;
				}
			}
#elif defined(NORMAL_ISO)
#endif

		}
		for (const auto& tempEdge : targetNodePointer->getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_SET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_SET(targetOut, nodeID);
			const bool i = IN_SET(targetIn, nodeID);
			const bool b = (o && i);
			bool temp = false;
			if (!o) {
				targetOut.insert(nodeID);
				targetMappingOutDepth[nodeID] = searchDepth;
				temp = i;
				if (i)targetBoth.insert(nodeID);
			}

			++targetMappingOutRefTimes[nodeID];
#ifdef INDUCE_ISO
			if (i) {
				if (!o) {
					if (targetMappingInDepth[nodeID] == searchDepth) ++targetC;
				}


			}
			if (temp == false && !o)++targetE;
#elif defined(NORMAL_ISO)
#endif
		}


		for (const auto& tempEdge : queryNodePointer->getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			//		queryMappingInRefTimes[sourceNodeID]++;
			const bool n = IN_SET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_SET(queryOut, nodeID);
			const bool i = IN_SET(queryIn, nodeID);
			const bool b = (o && i);

			if (!i) {
				queryIn.insert(nodeID);
				queryMappingInDepth[nodeID] = searchDepth;

				if (o)queryBoth.insert(nodeID);
			}

			++queryMappingInRefTimes[nodeID];
#ifdef INDUCE_ISO
			if (o) {
				//this node was not in inSet before add action
				if (!i) {
					++queryB;
				}
			}
#elif defined(NORMAL_ISO)
#endif
		}
		for (const auto& tempEdge : queryNodePointer->getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_SET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_SET(queryOut, nodeID);
			const bool i = IN_SET(queryIn, nodeID);
			const bool b = (o && i);
			bool temp = false;
			if (!o) {
				queryOut.insert(nodeID);
				queryMappingOutDepth[nodeID] = searchDepth;

				temp = i;
				if (i)queryBoth.insert(nodeID);
			}

			++queryMappingOutRefTimes[nodeID];
#ifdef INDUCE_ISO
			if (i) {
				if (!o) {
					if (queryMappingInDepth[nodeID] == searchDepth) ++queryC;
				}
			}
			if (temp == false && !o)++queryE;

#elif defined(NORMAL_ISO)
#endif
		}
#ifdef INDUCE_ISO
		const size_t targetInAdd = targetIn.size() - targetMappingInSizeOld,
			targetOutAdd = targetOut.size() - targetMappingOutSizeOld,
			targetBothAdd = targetBoth.size() - targetBothInOutSizeOld,
			queryInAdd = queryIn.size() - queryMappingInSizeOld,
			queryOutAdd = queryOut.size() - queryMappingOutSizeOld,
			queryBothAdd = queryBoth.size() - queryBothInOutSizeOld;

		queryA = queryInAdd - queryB - queryC;
		targetA = targetInAdd - targetB - targetC;
		queryD = queryOutAdd - queryC - queryE;
		targetD = targetOutAdd - targetC - targetE;
		//	if (induceGraph && (queryInAdd > targetInAdd || queryOutAdd > targetOutAdd  || queryBothAdd > targetBothAdd))stillConsistentAfterAdd = false;
		if ((queryA > targetA || queryE > targetE || queryC > targetC))stillConsistentAfterAdd = false;
		//	if (induceGraph && (queryA > targetA || queryE > targetE || queryC > targetC || queryB > targetB || queryD > targetD))stillConsistentAfterAdd = false;
		else stillConsistentAfterAdd = true;
#elif defined(NORMAL_ISO)
		if (queryIn.size() > targetIn.size() || queryOut.size() > targetIn.size() || queryBoth.size() > targetBoth.size()) stillConsistentAfterAdd = false;
		else stillConsistentAfterAdd = true;
#endif
		return;
	}
	void deleteCanditatePairToMapping(const MapPair& cp)
	{

		const auto& queryNodeID = cp.getKey();
		const auto& targetNodeID = cp.getValue();
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);

		for (const auto& tempEdge : queryNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();

			const bool n = IN_SET(queryUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_SET(queryBoth, nodeID);

			auto& refTimes = queryMappingInRefTimes[nodeID];
			auto& nodeDepth = queryMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryIn.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto& outRefTimes = queryMappingOutRefTimes[nodeID];
					queryBoth.erase(nodeID);
				}
				nodeDepth = 0;
			}
			else if (b) {
			}
			refTimes--;

		}
		for (const auto& tempEdge : queryNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();

			const bool n = IN_SET(queryUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_SET(queryBoth, nodeID);

			auto& refTimes = queryMappingOutRefTimes[nodeID];
			auto& nodeDepth = queryMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryOut.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto inRefTimes = queryMappingInRefTimes[nodeID];

					queryBoth.erase(nodeID);

				}
				nodeDepth = 0;
			}
			else if (b) {

			}

			refTimes--;
		}
		for (const auto& tempEdge : targetNode.getInEdges()) {
			const auto& nodeID = tempEdge.getSourceNodeID();
			const bool n = IN_SET(targetUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_SET(targetBoth, nodeID);

			auto& refTimes = targetMappingInRefTimes[nodeID];
			auto& nodeDepth = targetMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {

				targetIn.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto outRefTimes = targetMappingOutRefTimes[nodeID];
					targetBoth.erase(nodeID);

				}
				nodeDepth = 0;
			}
			else if (b) {
			}
			refTimes--;
		}
		for (const auto& tempEdge : targetNode.getOutEdges()) {
			const auto& nodeID = tempEdge.getTargetNodeID();
			const bool n = IN_SET(targetUnmap, nodeID);
			if (!n)continue;

			const bool b = IN_SET(targetBoth, nodeID);

			auto& refTimes = targetMappingOutRefTimes[nodeID];
			auto& nodeDepth = targetMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				targetOut.erase(nodeID);
				assert(refTimes == 1);

				if (b) {
					const auto inRefTimes = targetMappingInRefTimes[nodeID];

					targetBoth.erase(nodeID);
				}
				nodeDepth = 0;
			}
			else if (b) {
			}

			refTimes--;


		}
		if (queryMappingInDepth[queryNodeID]) {
			queryIn.insert(queryNodeID);

			const auto refTimes = queryMappingInRefTimes[queryNodeID];
			if (IN_SET(queryOut, queryNodeID)) {
				const auto outRefTimes = queryMappingOutRefTimes[queryNodeID];
				queryBoth.insert(queryNodeID);
			}

		}
		if (queryMappingOutDepth[queryNodeID])
		{
			queryOut.insert(queryNodeID);

			const auto refTimes = queryMappingOutRefTimes[queryNodeID];
			if (IN_SET(queryIn, queryNodeID)) {
				const auto inRefTimes = queryMappingInRefTimes[queryNodeID];
				queryBoth.insert(queryNodeID);
			}

		}
		if (targetMappingInDepth[targetNodeID]) {
			targetIn.insert(targetNodeID);

			const auto refTimes = targetMappingInRefTimes[targetNodeID];
			if (IN_SET(targetOut, targetNodeID)) {

				const auto outRefTimes = targetMappingOutRefTimes[targetNodeID];
				targetBoth.insert(targetNodeID);

			}

		}
		if (targetMappingOutDepth[targetNodeID])
		{
			targetOut.insert(targetNodeID);

			const auto refTimes = targetMappingOutRefTimes[targetNodeID];
			if (IN_SET(targetIn, targetNodeID)) {

				const auto inRefTimes = targetMappingInRefTimes[targetNodeID];

				targetBoth.insert(targetNodeID);
			}

		}

		mapping[queryNodeID] = NO_MAP;
		mappingAux[targetNodeID] = NO_MAP;
		targetUnmap.insert(targetNodeID);
		queryUnmap.insert(queryNodeID);
		searchDepth--;
		stillConsistentAfterAdd = true;
		return;

	}
	bool isCoverQueryGraph()const {
		if (queryGraph.size() == searchDepth)	return true;
		return false;
	}
	MapType getMap(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
};




