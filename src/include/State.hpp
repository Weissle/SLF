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

	NodeSetType targetUnmap, targetIn, targetOut, targetBoth,
		queryUnmap, queryIn, queryOut, queryBoth;

	vector<size_t> targetMappingInDepth, targetMappingOutDepth,
		queryMappingInDepth, queryMappingOutDepth;
	vector<size_t> targetMappingInRefTimes, targetMappingOutRefTimes,
		queryMappingInRefTimes, queryMappingOutRefTimes;

	//check the mapping is still consistent after add this pair
	bool sourceRule(const MapPair& cp)
	{
		const auto& querySourceNodeID = cp.first;
		const auto& targetSourceNodeID = cp.second;

		const auto& querySourceNode = queryGraph.node(querySourceNodeID);
		const auto& targetSourceNode = targetGraph.node(targetSourceNodeID);



		size_t inCount = 0, outCount = 0, bothCount = 0;
		size_t notTCount = 0;


		for (const auto& tempEdge : targetSourceNode.outEdges()) {
			const auto& targetTargetNodeID = tempEdge.target();

			if (!IN_NODE_SET(targetUnmap, targetTargetNodeID)) {
#ifdef INDUCE_ISO
				const auto queryTargetNodeID = mappingAux[targetTargetNodeID];
				const auto& queryTargetNode = queryGraph.node(queryTargetNodeID);
				if (querySourceNode.existSameTypeEdgeToNode(queryTargetNode, tempEdge) == false) return false;
#endif		
			}
			else if (targetTargetNodeID == targetSourceNodeID) {
#ifdef INDUCE_ISO
				if (querySourceNode.existSameTypeEdgeToNode(querySourceNode, tempEdge) == false)return false;
#endif
			}
			else {
				const bool o = IN_NODE_SET(targetOut, targetTargetNodeID);
				const bool i = IN_NODE_SET(targetIn, targetTargetNodeID);
				const bool b = (i && o);

				if (o && !b) {
					++outCount;
				}
				if (i && !b) {
					++inCount;
				}
				if (b) {
					++bothCount;
				}
			
				//maybe bug in normal_iso
				if (!i && !o) ++notTCount;
			}
		}
		for (const auto& tempEdge : querySourceNode.outEdges()) {
			const auto& queryTargetNodeID = tempEdge.target();
			//this tempnode have been mapped
			if (!IN_NODE_SET(queryUnmap, queryTargetNodeID)) {
				const auto targetTargetNodeID = mapping[queryTargetNodeID];
				const auto& targetTargetNode = targetGraph.node(targetTargetNodeID);
				if (targetSourceNode.existSameTypeEdgeToNode(targetTargetNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetSourceNode.existSameTypeEdgeToNode(targetSourceNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_NODE_SET(queryOut, queryTargetNodeID);
				const bool i = IN_NODE_SET(queryIn, queryTargetNodeID);
				const bool b = (o && i);

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
		const auto& queryTargetNodeID = cp.first;
		const auto& targetTargetNodeID = cp.second;

		const auto& queryTargetNode = queryGraph.node(queryTargetNodeID);
		const auto& targetTargetNode = targetGraph.node(targetTargetNodeID);


		size_t inCount = 0, outCount = 0, bothCount = 0;
		size_t notTCount = 0;


		for (const auto& tempEdge : targetTargetNode.inEdges()) {
			const auto& targetSourceNodeID = tempEdge.source();

			if (!IN_NODE_SET(targetUnmap, targetSourceNodeID)) {
#ifdef INDUCE_ISO
				const auto& querySourceNodeID = mappingAux[targetSourceNodeID];
				const auto& querySourceNode = queryGraph.node(querySourceNodeID);
				if (queryTargetNode.existSameTypeEdgeFromNode(querySourceNode, tempEdge) == false) return false;
#endif
			}
			else if (targetTargetNodeID == targetSourceNodeID) {
#ifdef INDUCE_ISO
				if (queryTargetNode.existSameTypeEdgeFromNode(queryTargetNode, tempEdge) == false)return false;
#endif
			}
			else {

				const bool o = IN_NODE_SET(targetOut, targetSourceNodeID);
				const bool i = IN_NODE_SET(targetIn, targetSourceNodeID);
				const bool b = (o && i);
				if (o && !b) {
					++outCount;
				}
				if (i && !b) {
					++inCount;
				}
				if (b) {
					++bothCount;
				}
				if (!i && !o) ++notTCount;
			}
		}


		for (const auto& tempEdge : queryTargetNode.inEdges()) {
			const auto& querySourceNodeID = tempEdge.source();
			if (!IN_NODE_SET(queryUnmap, querySourceNodeID)) {

				const auto& targetSourceNodeID = mapping[querySourceNodeID];
				const auto& targetSourceNode = targetGraph.node(targetSourceNodeID);
				if (targetTargetNode.existSameTypeEdgeFromNode(targetSourceNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetTargetNode.existSameTypeEdgeFromNode(targetTargetNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_NODE_SET(queryOut, querySourceNodeID);
				const bool i = IN_NODE_SET(queryIn, querySourceNodeID);
				const bool b = (o && i);
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

				if (!i && !o) {
					if (notTCount)--notTCount;
					else return false;
				}
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
		targetBoth.containerClone(targetIn);
		targetUnmap.containerClone(targetIn);

		queryIn = NodeSetType(queryGraph);
		queryOut.containerClone(queryIn);
		queryBoth.containerClone(queryIn);
		queryUnmap.containerClone(queryIn);

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

		/*	if (queryNodeInIn && queryNodeInOut) tempNodeSetPointer = &targetBoth[queryNodeLabel];
			else*/ if (queryNodeInIn) tempNodeSetPointer = &targetIn[queryNodeLabel];
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

		//	return answer;
			return std::move(answer);

	}
	bool checkCanditatePairIsAddable(const MapPair& cp)
	{
		const bool answer = sourceRule(cp) && targetRule(cp);
		//	cout << answer << endl;
		return answer;
	}
	void addCanditatePairToMapping(const MapPair& cp)
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
			const bool n = (i)? true : IN_NODE_SET(targetUnmap, nodeID);
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
			const bool n =(o)? true : IN_NODE_SET(targetUnmap, nodeID);
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
			const bool n = (i)? true : IN_NODE_SET(queryUnmap, nodeID);
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
			const bool n = (o)? true : IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			if (!o) {
				queryOut.insert(nodeID);
				queryMappingOutDepth[nodeID] = searchDepth;
			}
			++queryMappingOutRefTimes[nodeID];

		}
		return;
	}
	void deleteCanditatePairToMapping(const MapPair& cp)
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