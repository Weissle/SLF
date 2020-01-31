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
#ifdef INDUCE_ISO
	int** DRin, ** DRout;
#endif
	inline void seePairID(const MapPair& cp)const {
		cout << "Ready To Mapping" << "(" << cp.first << "," << cp.second << ")" << endl;
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

	//check the mapping is still consistent after add this pair
	bool sourceRule(const MapPair& cp)
	{
		const auto& querySourceNodeID = cp.first;
		const auto& targetSourceNodeID = cp.second;

		const auto& querySourceNode = queryGraph.getNode(querySourceNodeID);
		const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);


#ifdef INDUCE_ISO
		typedef FSPair<size_t, size_t> DRPair;
		unordered_map< FSPair< DRPair, DRPair>, size_t>  bothIO;
		bothIO.reserve(calHashSuitableSize(targetSourceNode.outEdgesNum()));
		clear_two_dim_array(DRin, searchDepth + 1);
		clear_two_dim_array(DRout, searchDepth + 1);

#elif defined(NORMAL_ISO)
		size_t inCount = 0, outCount = 0, bothCount = 0;
#endif
		size_t notTCount = 0;


		for (const auto& tempEdge : targetSourceNode.outEdges()) {
			const auto& targetTargetNodeID = tempEdge.target();

			if (!IN_NODE_SET(targetUnmap, targetTargetNodeID)) {
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

				const bool o = IN_NODE_SET(targetOut, targetTargetNodeID);
				const bool i = IN_NODE_SET(targetIn, targetTargetNodeID);
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
				//maybe bug in normal_iso
				if (!i && !o) ++notTCount;
			}
		}
		for (const auto& tempEdge : querySourceNode.outEdges()) {
			const auto& queryTargetNodeID = tempEdge.target();
			//this tempnode have been mapped
			if (!IN_NODE_SET(queryUnmap, queryTargetNodeID)) {
				const auto targetTargetNodeID = mapping[queryTargetNodeID];
				const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);
				if (targetSourceNode.existSameTypeEdgeToNode(targetTargetNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetSourceNode.existSameTypeEdgeToNode(targetSourceNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_NODE_SET(queryOut, queryTargetNodeID);
				const bool i = IN_NODE_SET(queryIn, queryTargetNodeID);
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
		const auto& queryTargetNodeID = cp.first;
		const auto& targetTargetNodeID = cp.second;

		const auto& queryTargetNode = queryGraph.getNode(queryTargetNodeID);
		const auto& targetTargetNode = targetGraph.getNode(targetTargetNodeID);

#ifdef INDUCE_ISO
		typedef FSPair<size_t, size_t> DRPair;
		unordered_map< FSPair< DRPair, DRPair>, size_t>  bothIO;
		bothIO.reserve(calHashSuitableSize(targetTargetNode.inEdgesNum()));
		clear_two_dim_array(DRin, searchDepth + 1);
		clear_two_dim_array(DRout, searchDepth + 1);

#elif defined(NORMAL_ISO)
		size_t inCount = 0, outCount = 0, bothCount = 0;
#endif
		size_t notTCount = 0;


		for (const auto& tempEdge : targetTargetNode.inEdges()) {
			const auto& targetSourceNodeID = tempEdge.source();

			if (!IN_NODE_SET(targetUnmap, targetSourceNodeID)) {
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

				const bool o = IN_NODE_SET(targetOut, targetSourceNodeID);
				const bool i = IN_NODE_SET(targetIn, targetSourceNodeID);
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


		for (const auto& tempEdge : queryTargetNode.inEdges()) {
			const auto& querySourceNodeID = tempEdge.source();
			if (!IN_NODE_SET(queryUnmap, querySourceNodeID)) {

				const auto& targetSourceNodeID = mapping[querySourceNodeID];
				const auto& targetSourceNode = targetGraph.getNode(targetSourceNodeID);
				if (targetTargetNode.existSameTypeEdgeFromNode(targetSourceNode, tempEdge) == false) return false;
			}
			else if (queryTargetNodeID == querySourceNodeID) {
				if (targetTargetNode.existSameTypeEdgeFromNode(targetTargetNode, tempEdge) == false)return false;
			}
			else {

				const bool o = IN_NODE_SET(queryOut, querySourceNodeID);
				const bool i = IN_NODE_SET(queryIn, querySourceNodeID);
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
#ifdef INDUCE_ISO
		new_two_dim_array(DRin, queryGraphSize + 1);
		new_two_dim_array(DRout, queryGraphSize + 1);

#elif defined(NORMAL_ISO)
#endif

	};
	State() = default;
	~State() {
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

		const auto& queryNodeToMatchID = id;
		const bool queryNodeInIn = IN_NODE_SET(queryIn, queryNodeToMatchID);
		const bool queryNodeInOut = IN_NODE_SET(queryOut, queryNodeToMatchID);
		const auto& queryNode = queryGraph.getNode(queryNodeToMatchID);
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
				const auto& targetNode = targetGraph.getNode(targetNodeToMatchID);
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
		const auto targetNodeID = cp.second;
		const auto queryNodeID = cp.first;
		mapping[queryNodeID] = targetNodeID;
		mappingAux[targetNodeID] = queryNodeID;


		targetUnmap.erase(targetNodeID);
		queryUnmap.erase(queryNodeID);

		targetIn.erase(targetNodeID);
		targetOut.erase(targetNodeID);
		//		targetBoth.erase(targetNodeID);
		queryIn.erase(queryNodeID);
		queryOut.erase(queryNodeID);
		//		queryBoth.erase(queryNodeID);



		const auto& targetNodePointer = targetGraph.getNodePointer(targetNodeID);
		const auto& queryNodePointer = queryGraph.getNodePointer(queryNodeID);
		searchDepth++;

		// there is a variables' name using error about [In/Out]RefTimesCla
		// like below  inEdges should use targetOutRefTimesCla to record info
		// but for coding convenience , I use targetInRefTimesCla 
		// so you can see the swap function in the construct function .(??? Where is the swap function ?? I forget ...)
		for (const auto& tempEdge : targetNodePointer->inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODE_SET(targetOut, nodeID);
			const bool i = IN_NODE_SET(targetIn, nodeID);
			const bool b = (o && i);
			if (!i) {
				targetIn.insert(nodeID);
				targetMappingInDepth[nodeID] = searchDepth;
				//				if (o)targetBoth.insert(nodeID);
			}
			++targetMappingInRefTimes[nodeID];
		}
		for (const auto& tempEdge : targetNodePointer->outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODE_SET(targetOut, nodeID);
			const bool i = IN_NODE_SET(targetIn, nodeID);
			const bool b = (o && i);
			if (!o) {
				targetOut.insert(nodeID);
				targetMappingOutDepth[nodeID] = searchDepth;
				//				if (i)targetBoth.insert(nodeID);
			}
			++targetMappingOutRefTimes[nodeID];
		}


		for (const auto& tempEdge : queryNodePointer->inEdges()) {
			const auto& nodeID = tempEdge.source();
			//		queryMappingInRefTimes[sourceNodeID]++;
			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODE_SET(queryOut, nodeID);
			const bool i = IN_NODE_SET(queryIn, nodeID);
			const bool b = (o && i);

			if (!i) {
				queryIn.insert(nodeID);
				queryMappingInDepth[nodeID] = searchDepth;
				//				if (o)queryBoth.insert(nodeID);
			}
			++queryMappingInRefTimes[nodeID];

		}
		for (const auto& tempEdge : queryNodePointer->outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			const bool o = IN_NODE_SET(queryOut, nodeID);
			const bool i = IN_NODE_SET(queryIn, nodeID);
			const bool b = (o && i);
			if (!o) {
				queryOut.insert(nodeID);
				queryMappingOutDepth[nodeID] = searchDepth;
				//				if (i)queryBoth.insert(nodeID);
			}
			++queryMappingOutRefTimes[nodeID];

		}
		return;
	}
	void deleteCanditatePairToMapping(const MapPair& cp)
	{

		const auto& queryNodeID = cp.first;
		const auto& targetNodeID = cp.second;
		const auto& queryNode = queryGraph.getNode(queryNodeID);
		const auto& targetNode = targetGraph.getNode(targetNodeID);

		for (const auto& tempEdge : queryNode.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;
			//		const bool b = IN_NODE_SET(queryBoth, nodeID);
			auto& refTimes = queryMappingInRefTimes[nodeID];
			auto& nodeDepth = queryMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryIn.erase(nodeID);
				/*		if (b) {
			//				queryBoth.erase(nodeID);
						}*/
				nodeDepth = 0;
			}
			refTimes--;

		}
		for (const auto& tempEdge : queryNode.outEdges()) {
			const auto& nodeID = tempEdge.target();

			const bool n = IN_NODE_SET(queryUnmap, nodeID);
			if (!n)continue;

			//		const bool b = IN_NODE_SET(queryBoth, nodeID);

			auto& refTimes = queryMappingOutRefTimes[nodeID];
			auto& nodeDepth = queryMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				queryOut.erase(nodeID);
				/*		if (b) {
				//			queryBoth.erase(nodeID);
						}*/
				nodeDepth = 0;
			}
			refTimes--;
		}
		for (const auto& tempEdge : targetNode.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			//	const bool b = IN_NODE_SET(targetBoth, nodeID);
			auto& refTimes = targetMappingInRefTimes[nodeID];
			auto& nodeDepth = targetMappingInDepth[nodeID];
			if (nodeDepth == searchDepth) {
				targetIn.erase(nodeID);
				/*		if (b) {
				//			targetBoth.erase(nodeID);
						}*/
				nodeDepth = 0;
			}
			refTimes--;
		}
		for (const auto& tempEdge : targetNode.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(targetUnmap, nodeID);
			if (!n)continue;
			//	const bool b = IN_NODE_SET(targetBoth, nodeID);

			auto& refTimes = targetMappingOutRefTimes[nodeID];
			auto& nodeDepth = targetMappingOutDepth[nodeID];
			if (nodeDepth == searchDepth) {
				targetOut.erase(nodeID);
				/*		if (b) {
				//			targetBoth.erase(nodeID);
						}*/
				nodeDepth = 0;
			}
			refTimes--;
		}
		if (queryMappingInDepth[queryNodeID]) {
			queryIn.insert(queryNodeID);
			const auto refTimes = queryMappingInRefTimes[queryNodeID];
			if (IN_NODE_SET(queryOut, queryNodeID)) {
				const auto outRefTimes = queryMappingOutRefTimes[queryNodeID];
				//		queryBoth.insert(queryNodeID);
			}
		}
		if (queryMappingOutDepth[queryNodeID])
		{
			queryOut.insert(queryNodeID);
			const auto refTimes = queryMappingOutRefTimes[queryNodeID];
			if (IN_NODE_SET(queryIn, queryNodeID)) {
				const auto inRefTimes = queryMappingInRefTimes[queryNodeID];
				//		queryBoth.insert(queryNodeID);
			}
		}
		if (targetMappingInDepth[targetNodeID]) {
			targetIn.insert(targetNodeID);
			const auto refTimes = targetMappingInRefTimes[targetNodeID];
			if (IN_NODE_SET(targetOut, targetNodeID)) {
				const auto outRefTimes = targetMappingOutRefTimes[targetNodeID];
				//		targetBoth.insert(targetNodeID);
			}
		}
		if (targetMappingOutDepth[targetNodeID])
		{
			targetOut.insert(targetNodeID);
			const auto refTimes = targetMappingOutRefTimes[targetNodeID];
			if (IN_NODE_SET(targetIn, targetNodeID)) {

				const auto inRefTimes = targetMappingInRefTimes[targetNodeID];

				//		targetBoth.insert(targetNodeID);
			}
		}
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