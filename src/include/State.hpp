#pragma once
#include<vector>
#include<time.h>
#include"Graph.hpp"
#include"Pair.hpp"
#include"NodeSet.hpp"
#include<assert.h>
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<limits>
#include"common.h"
#include<si_marcos.h>
#include<cstring>

//defind INDUCE_ISO or NORMAL_ISO in si_marcos.h
#if !defined(INDUCE_ISO) && !defined(NORMAL_ISO)
#error  you should defind INDUCE_ISO or NORMAL_ISO
#endif
#if defined(INDUCE_ISO) && defined(NORMAL_ISO)
#error you should not defind INDUCE_ISO and NORMAL_ISO at the same time
#endif


using namespace std;

namespace wg {

template<typename _GraphType>
class State {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;

	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef const NodeType* NodeCPointer;
	typedef pair<NodeIDType, NodeIDType> MapPair;
	typedef vector<NodeIDType> MapType;

	typedef NodeSetWithLabel<GraphType> NodeSetType;


private:
	const GraphType& targetGraph, & queryGraph;
	size_t searchDepth = 0;
	MapType mapping;
	MapType mappingAux; //from query to target

	NodeSetType targetUnmap, targetIn, targetOut,
		queryUnmap, queryIn, queryOut;

	vector<size_t> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;
	vector<size_t> targetMappingInRefTimes, targetMappingOutRefTimes,
		queryMappingInRefTimes, queryMappingOutRefTimes;
	size_t labelTypeNum;

	vector<size_t> inNewCount, outNewCount, bothNewCount, notNewCount;
	//used in look forward 2 
	void clearNewCount() {
		const auto temp = sizeof(size_t) * labelTypeNum;
		memset(inNewCount.data(), 0, temp);
		memset(outNewCount.data(), 0, temp);
		memset(bothNewCount.data(), 0, temp);
#ifdef INDUCE_ISO
		memset(notNewCount.data(), 0, temp);
#endif
	}
	//check the mapping is still consistent after add this pair
	bool sourceRule(const MapPair& cp)
	{
		const auto& querySourceNodeID = cp.first;
		const auto& targetSourceNodeID = cp.second;

		const auto& querySourceNode = queryGraph.node(querySourceNodeID);
		const auto& targetSourceNode = targetGraph.node(targetSourceNodeID);

		clearNewCount();
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map
		for (const auto& tempEdge : targetSourceNode.outEdges()) {
			const auto& targetTargetNodeID = tempEdge.target();
			const bool notMapped = IN_NODE_SET(targetUnmap, targetTargetNodeID);
			if (notMapped && targetTargetNodeID != targetSourceNodeID) {
				const bool o = IN_NODE_SET(targetOut, targetTargetNodeID);
				const bool i = IN_NODE_SET(targetIn, targetTargetNodeID);
				const bool b = (i && o);
				const auto label = targetGraph[targetTargetNodeID].label();
				if (b) 	bothNewCount[label]++;
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;
#ifdef NORMAL_ISO
			}
#elif defined(INDUCE_ISO)
				else ++notNewCount[label];
			}
			else {
				const auto queryTargetNodeID = (notMapped) ? querySourceNodeID : mappingAux[targetTargetNodeID];
				if (queryGraph.existEdge(querySourceNodeID, queryTargetNodeID, tempEdge.label()) == false) return false;
			}
#endif
		}

		for (const auto& tempEdge : querySourceNode.outEdges()) {
			const auto& queryTargetNodeID = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = IN_NODE_SET(queryUnmap, queryTargetNodeID);
			if (notMapped && queryTargetNodeID != querySourceNodeID) {
				const bool o = IN_NODE_SET(queryOut, queryTargetNodeID);
				const bool i = IN_NODE_SET(queryIn, queryTargetNodeID);
				const bool b = (o && i);
				const auto label = queryGraph[queryTargetNodeID].label();
				if (b) {
					if (bothNewCount[label]--);
					else return false;
				}
				else if (o) {
					if (outNewCount[label]--);
					else return false;
				}
				else if (i) {
					if (inNewCount[label]--);
					else return false;
				}
#if defined(INDUCE_ISO)
				else {
					if (notNewCount[label]--);
					else return false;
				}
#endif
			}
			else {
				const auto targetTargetNodeID = (notMapped) ? targetSourceNodeID : mapping[queryTargetNodeID];
				if (targetGraph.existEdge(targetSourceNodeID, targetTargetNodeID, tempEdge.label()) == false)return false;
			}

		}

		return true;

	}
	bool targetRule(const MapPair& cp)
	{
		const auto& queryTargetNodeID = cp.first;
		const auto& targetTargetNodeID = cp.second;

		const auto& queryTargetNode = queryGraph.node(queryTargetNodeID);
		const auto& targetTargetNode = targetGraph.node(targetTargetNodeID);

		clearNewCount();
		for (const auto& tempEdge : targetTargetNode.inEdges()) {
			const auto& targetSourceNodeID = tempEdge.source();
			const bool notMapped = IN_NODE_SET(targetUnmap, targetSourceNodeID);
			if (notMapped && targetTargetNodeID != targetSourceNodeID) {
				const bool o = IN_NODE_SET(targetOut, targetSourceNodeID);
				const bool i = IN_NODE_SET(targetIn, targetSourceNodeID);
				const bool b = (o && i);
				const auto label = targetGraph[targetSourceNodeID].label();
				if (b) 	bothNewCount[label]++;
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;
#ifdef NORMAL_ISO
			}
#elif defined(INDUCE_ISO)
				else ++notNewCount[label];
			}
			else {
				const auto querySourceNodeID = (notMapped) ? queryTargetNodeID : mappingAux[targetSourceNodeID];
				if (queryGraph.existEdge(querySourceNodeID, queryTargetNodeID, tempEdge.label()) == false)return false;
			}
#endif
		}

		for (const auto& tempEdge : queryTargetNode.inEdges()) {
			const auto& querySourceNodeID = tempEdge.source();
			const bool notMapped = IN_NODE_SET(queryUnmap, querySourceNodeID);
			if (notMapped && queryTargetNodeID != querySourceNodeID) {
				const bool o = IN_NODE_SET(queryOut, querySourceNodeID);
				const bool i = IN_NODE_SET(queryIn, querySourceNodeID);
				const bool b = (o && i);
				const auto label = queryGraph[querySourceNodeID].label();
				if (b) {
					if (bothNewCount[label]--);
					else return false;
				}
				else if (o) {
					if (outNewCount[label]--);
					else return false;
				}
				else if (i) {
					if (inNewCount[label]--);
					else return false;
				}
#if defined(INDUCE_ISO)
				else {
					if (notNewCount[label]--);
					else return false;
				}
#endif
			}
			else {
				const auto targetSourceNodeID = (notMapped) ? targetTargetNodeID : mapping[querySourceNodeID];
				if (targetGraph.existEdge(targetSourceNodeID, targetTargetNodeID, tempEdge.label()) == false)return false;
			}	
		}
		return true;
	}

public:
	State(const GraphType& _q, const GraphType& _t) :queryGraph(_q), targetGraph(_t) {

		const auto queryGraphSize = queryGraph.size();
		const auto targetGraphSize = targetGraph.size();
		const auto queryHashSize = calHashSuitableSize(queryGraphSize);
		const auto targetHashSize = calHashSuitableSize(targetGraphSize);

		mappingAux.resize(targetGraphSize);
		for (auto& i : mappingAux) i = NO_MAP;
		mapping.resize(queryGraphSize);
		for (auto& i : mapping) i = NO_MAP;

		targetIn = NodeSetType(targetGraph);
		targetOut.containerClone(targetIn);
		targetUnmap.containerClone(targetIn);

		queryIn = NodeSetType(queryGraph);
		queryOut.containerClone(queryIn);
		queryUnmap.containerClone(queryIn);

		targetMappingInDepth.resize(targetGraphSize);
		targetMappingOutDepth.resize(targetGraphSize);
		queryMappingInDepth.resize(queryGraphSize);
		queryMappingOutDepth.resize(queryGraphSize);


		targetMappingInRefTimes.resize(targetGraphSize);
		targetMappingOutRefTimes.resize(targetGraphSize);
		queryMappingInRefTimes.resize(queryGraphSize);
		queryMappingOutRefTimes.resize(queryGraphSize);

		labelTypeNum = max(queryGraph.LQinform().size(), targetGraph.LQinform().size());
		inNewCount.resize(labelTypeNum);
		outNewCount.resize(labelTypeNum);
		bothNewCount.resize(labelTypeNum);
		notNewCount.resize(labelTypeNum);
		for (const auto& node : queryGraph.nodes()) queryUnmap.insert(node.id());
		for (const auto& node : targetGraph.nodes()) targetUnmap.insert(node.id());


	};
	State() = default;
	~State() {
	}

public:
	//public function
	vector<MapPair> calCandidatePairs(const NodeIDType id)const
	{
		vector<MapPair> answer;

		const auto& queryNodeToMatchID = id;
		const bool queryNodeInIn = IN_NODE_SET(queryIn, queryNodeToMatchID);
		const bool queryNodeInOut = IN_NODE_SET(queryOut, queryNodeToMatchID);
		const auto& queryNode = queryGraph.node(queryNodeToMatchID);
		const auto queryNodeInRefTimes = queryMappingInRefTimes[queryNodeToMatchID];
		const auto queryNodeOutRefTimes = queryMappingOutRefTimes[queryNodeToMatchID];
		const auto queryNodeInDepth = queryMappingInDepth[queryNodeToMatchID];
		const auto queryNodeOutDepth = queryMappingOutDepth[queryNodeToMatchID];



		const auto queryNodeLabel = queryNode.label();

		const unordered_set<NodeIDType>* tempNodeSetPointer;

		if (queryNodeInIn) tempNodeSetPointer = &targetIn[queryNodeLabel];
		else if (queryNodeInOut)tempNodeSetPointer = &targetOut[queryNodeLabel];
		else tempNodeSetPointer = &targetUnmap[queryNodeLabel];

		answer.reserve(tempNodeSetPointer->size());
		const auto& targetNodeToMatchSet = *tempNodeSetPointer;

		TRAVERSE_SET(targetNodeToMatchID, targetNodeToMatchSet)
		{
			const auto& targetNode = targetGraph.node(targetNodeToMatchID);
			if (/*queryNode.isSameType(targetNode) == false || */queryNode > targetNode) continue;

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

		return std::move(answer);

	}
	bool checkCanditatePairIsAddable(const MapPair & cp)
	{
		const bool answer = sourceRule(cp) && targetRule(cp);
		return answer;
	}
	void addCanditatePairToMapping(const MapPair & cp)
	{
		const auto targetNodeID = cp.second;
		const auto queryNodeID = cp.first;
		mapping[queryNodeID] = targetNodeID;
		mappingAux[targetNodeID] = queryNodeID;


		targetUnmap.erase(targetNodeID);
		queryUnmap.erase(queryNodeID);

		targetIn.erase(targetNodeID);
		targetOut.erase(targetNodeID);

		queryIn.erase(queryNodeID);
		queryOut.erase(queryNodeID);

		const auto& targetNodePointer = targetGraph.nodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.nodePointer(queryNodeID);
		searchDepth++;

		// there is a variables' name using error about [In/Out]RefTimesCla
		// like below  inEdges should use targetOutRefTimesCla to record info
		// but for coding convenience , I use targetInRefTimesCla 
		// so you can see the swap function in the construct function .(??? Where is the swap function ?? I forget ...)
		for (const auto& tempEdge : targetNodePointer->inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool i = IN_NODE_SET(targetIn, nodeID);
			const bool n = (i) ? true : IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			if (!i) {
				targetIn.insert(nodeID);
				targetMappingInDepth[nodeID] = searchDepth;
			}
			++targetMappingInRefTimes[nodeID];
		}
		for (const auto& tempEdge : targetNodePointer->outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = IN_NODE_SET(targetOut, nodeID);
			const bool n = (o) ? true : IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			if (!o) {
				targetOut.insert(nodeID);
				targetMappingOutDepth[nodeID] = searchDepth;
			}
			++targetMappingOutRefTimes[nodeID];
		}


		for (const auto& tempEdge : queryNodePointer->inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool i = IN_NODE_SET(queryIn, nodeID);
			const bool n = (i) ? true : IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			if (!i) {
				queryIn.insert(nodeID);
				queryMappingInDepth[nodeID] = searchDepth;
			}
			++queryMappingInRefTimes[nodeID];

		}
		for (const auto& tempEdge : queryNodePointer->outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = IN_NODE_SET(queryOut, nodeID);
			const bool n = (o) ? true : IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			if (!o) {
				queryOut.insert(nodeID);
				queryMappingOutDepth[nodeID] = searchDepth;
			}
			++queryMappingOutRefTimes[nodeID];

		}
		return;
	}
	void deleteCanditatePairToMapping(const MapPair & cp)
	{

		const auto& queryNodeID = cp.first;
		const auto& targetNodeID = cp.second;
		const auto& queryNode = queryGraph.node(queryNodeID);
		const auto& targetNode = targetGraph.node(targetNodeID);

		for (const auto& tempEdge : queryNode.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			auto& refTimes = queryMappingInRefTimes[nodeID];
			auto& nodeDepth = queryMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryIn.erase(nodeID);
				nodeDepth = 0;
			}
			refTimes--;

		}
		for (const auto& tempEdge : queryNode.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			auto& refTimes = queryMappingOutRefTimes[nodeID];
			auto& nodeDepth = queryMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryOut.erase(nodeID);
				nodeDepth = 0;
			}
			refTimes--;
		}
		for (const auto& tempEdge : targetNode.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			auto& refTimes = targetMappingInRefTimes[nodeID];
			auto& nodeDepth = targetMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				targetIn.erase(nodeID);
				nodeDepth = 0;
			}
			refTimes--;
		}
		for (const auto& tempEdge : targetNode.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			auto& refTimes = targetMappingOutRefTimes[nodeID];
			auto& nodeDepth = targetMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				targetOut.erase(nodeID);
				nodeDepth = 0;
			}
			refTimes--;
		}
		if (queryMappingInDepth[queryNodeID])	queryIn.insert(queryNodeID);
		if (queryMappingOutDepth[queryNodeID])	queryOut.insert(queryNodeID);
		if (targetMappingInDepth[targetNodeID])	targetIn.insert(targetNodeID);
		if (targetMappingOutDepth[targetNodeID])	targetOut.insert(targetNodeID);
		mapping[queryNodeID] = NO_MAP;
		mappingAux[targetNodeID] = NO_MAP;
		targetUnmap.insert(targetNodeID);
		queryUnmap.insert(queryNodeID);
		searchDepth--;

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

}