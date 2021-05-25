#pragma once
#include <bits/stdint-uintn.h>
#include <vector>
#include <time.h>
#include "graph/Graph.hpp"
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <memory>
#include "common.h"
#include <si/si_marcos.h>
#include <cstring>
#include "si/Tasks.hpp"
//defind INDUCE_ISO or NORMAL_ISO in si_marcos.h
#if !defined(INDUCE_ISO) && !defined(NORMAL_ISO)
#error  you should defind INDUCE_ISO or NORMAL_ISO
#endif
#if defined(INDUCE_ISO) && defined(NORMAL_ISO)
#error you should not defind INDUCE_ISO and NORMAL_ISO at the same time
#endif

using namespace std;

namespace wg {
template<typename EdgeLabel>
class SubgraphMatchStates {
	//	size_t search_depth = 0;
	using GraphType = GraphS<EdgeLabel>;
	const GraphType* graphPointer;
	vector<size_t> in_depth, out_depth;
	vector<NodeIDType> match_sequence;
	const vector<size_t> match_depth;	// a node's depth after it add to state , from 1 to n (graphPointer - > size()) ; 
	
	SubgraphMatchStates() = default;
public:
	SubgraphMatchStates(const GraphType& g, const vector<size_t>& _match_depth,const vector<NodeIDType>& _match_sequence) :graphPointer(&g), match_depth(_match_depth),
		in_depth(g.Size()), out_depth(g.Size()),match_sequence(_match_sequence)
	{}
	void addNode(const NodeIDType id, const size_t search_depth) {
		//const GraphType& graph = *graphPointer;
		//		search_depth++;
		//const auto& node = graph.node(id);
		//for (const auto& tempEdge : node.inEdges()) {
		for (const auto& tempEdge : graphPointer -> GetInEdges(id)) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped and not in set in
			if (!inSetIn(nodeID, search_depth) && inSetUnmap(nodeID, search_depth)) 	in_depth[nodeID] = search_depth;
		}
		for (const auto& tempEdge : graphPointer -> GetOutEdges(id)) {
			const auto& nodeID = tempEdge.target();
			if (!inSetOut(nodeID, search_depth) && inSetUnmap(nodeID, search_depth)) 	out_depth[nodeID] = search_depth;
		}
		return;
	}
	inline size_t inDepth(const NodeIDType id, const size_t search_depth)const {
		if (in_depth[id] <= search_depth)return in_depth[id];
		else return 0;
	}
	inline size_t outDepth(const NodeIDType id, const size_t search_depth)const {
		if (out_depth[id] <= search_depth)return out_depth[id];
		else return 0;
	}
	inline bool inSetIn(const NodeIDType id, const size_t search_depth)const { return inSetUnmap(id, search_depth) && inDepth(id, search_depth); }
	inline bool inSetOut(const NodeIDType id, const size_t search_depth)const { return inSetUnmap(id, search_depth) && outDepth(id, search_depth); }
	inline bool inSetUnmap(const NodeIDType id, const size_t search_depth)const { return match_depth[id] > search_depth; }
	inline size_t matchDepth(const NodeIDType id)const { return match_depth[id]; }
	inline NodeIDType matchID(const size_t depth)const { return match_sequence[depth]; }
};

template<typename EdgeLabel>
class State {
	using GraphType = GraphS<EdgeLabel>;
private:
	vector<size_t> in_depth, out_depth;
	shared_ptr<const SubgraphMatchStates<EdgeLabel>> queryStates;
	const GraphType* targetGraphPtr, * queryGraphPtr;
	size_t search_depth = 0;
	MapType mapping;
	MapType mappingAux; //from target to query

	vector<int> inNewCount, outNewCount, bothNewCount, notNewCount;
	//used in look forward 2 
	void clearNewCount() {
		for (auto i = 0; i < inNewCount.size(); ++i) {
			inNewCount[i] = 0; 
			outNewCount[i] = 0;
			bothNewCount[i] = 0; 
#ifdef INDUCE_ISO
			notNewCount[i] = 0;
#endif
		}
	}
	bool checkNewCount()const {
		for (auto i = 0; i < inNewCount.size(); ++i) {
			if (inNewCount[i] < 0 || outNewCount[i] < 0 || bothNewCount[i] < 0 || notNewCount[i] < 0)return false;
		}
		return true;
	}
	//check the mapping is still consistent after add this pair

	bool induceCheck(const NodeIDType& query_id, const NodeIDType& target_id) {
		//const GraphType& targetGraph = *targetGraphPtr;
		//const GraphType& queryGraph = *queryGraphPtr;

		//const auto& query_node = queryGraph.node(query_id);
		//const auto& target_node = targetGraph.node(target_id);

		int out_edge_inmap_num = 0, in_edge_inmap_num = 0;
		clearNewCount();
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map

		//for (const auto& tempEdge : target_node.outEdges()) {
		for (const auto& tempEdge : targetGraphPtr->GetOutEdges(target_id)) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = inSetUnmap(target_toid);
			if (notMapped && target_toid != target_id) {
				const bool o = inSetOut(target_toid);
				const bool i = inSetIn(target_toid);
				const bool b = (i && o);
				//const auto label = targetGraph[target_toid].label();
				const auto label = targetGraphPtr->GetLabel(target_toid);
				if (b) 	bothNewCount[label]++;
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;
				else ++notNewCount[label];
			}
			else {
				++out_edge_inmap_num;
				const auto query_toid = (notMapped) ? query_id : mappingAux[target_toid];
				if (queryGraphPtr->ExistEdge(query_id, query_toid, tempEdge.label()) == false) return false;
			}
		}

		for (const auto& tempEdge : queryGraphPtr->GetOutEdges(query_id)) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = queryStates->inSetUnmap(query_toid, search_depth);
			if (notMapped && query_toid != query_id) {
				const bool o = queryStates->inSetOut(query_toid, search_depth);
				const bool i = queryStates->inSetIn(query_toid, search_depth);
				const bool b = (o && i);
				//const auto label = queryGraph[query_toid].label();
				const auto label = queryGraphPtr->GetLabel(query_toid);
				if (b) { bothNewCount[label]--; }
				else if (o) { outNewCount[label]--; }
				else if (i) { inNewCount[label]--; }
				else {notNewCount[label]--;}
			}
			else --out_edge_inmap_num;

		}
		if (out_edge_inmap_num) return false;
		if (checkNewCount() == false) {
	//		++cnc;
			return false;
		}
		clearNewCount();

		for (const auto& tempEdge : targetGraphPtr->GetInEdges(target_id)) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = inSetUnmap(target_fromid);
			if (notMapped && target_id != target_fromid) {
				const bool o = inSetOut(target_fromid);
				const bool i = inSetIn(target_fromid);
				const bool b = (o && i);
				//const auto label = targetGraph[target_fromid].label();
				const auto label = targetGraphPtr->GetLabel(target_fromid);
				if (b) 	bothNewCount[label]++;
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
			}
			else {
				++in_edge_inmap_num;
				const auto query_fromid = (notMapped) ? query_id : mappingAux[target_fromid];
				if (queryGraphPtr->ExistEdge(query_fromid, query_id, tempEdge.label()) == false)return false;
			}
		}

		for (const auto& tempEdge : queryGraphPtr->GetInEdges(query_id)) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = queryStates->inSetUnmap(query_fromid, search_depth);
			if (notMapped && query_id != query_fromid) {
				const bool o = queryStates->inSetOut(query_fromid, search_depth);
				const bool i = queryStates->inSetIn(query_fromid, search_depth);
				const bool b = (o && i);
				//const auto label = queryGraph[query_fromid].label();
				const auto label = queryGraphPtr->GetLabel(query_fromid);
				if (b) { bothNewCount[label]--; }
				else if (o) { outNewCount[label]--; }
				else if (i) { inNewCount[label]--; }
				else { notNewCount[label]--; }
			}
			else --in_edge_inmap_num;	
		}
		if (in_edge_inmap_num)return false;
		if (checkNewCount() == false) {
	//		cnc++;
			return false;
		}
		return true;
	}

	bool normalCheck(const NodeIDType& query_id, const NodeIDType& target_id) {
		clearNewCount();

		//for (const auto& tempEdge : target_node.outEdges()) {
		for (const auto& tempEdge : targetGraphPtr->GetOutEdges(target_id)) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = inSetUnmap(target_toid);
			if (notMapped && target_toid != target_id) {
				const bool o = inSetOut(target_toid);
				const bool i = inSetIn(target_toid);
				const bool b = (i && o);
				//const auto label = targetGraph[target_toid].label();
				const auto label = targetGraphPtr->GetLabel(target_toid);
				if (b) {
					bothNewCount[label]++;
					outNewCount[label]++;
					inNewCount[label]++;
				}
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;
				else ++notNewCount[label];
			}
		}

		//for (const auto& tempEdge : query_node.outEdges()) {
		for (const auto& tempEdge : queryGraphPtr->GetOutEdges(query_id)) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = queryStates->inSetUnmap(query_toid,search_depth);
			if (notMapped && query_toid != query_id) {
				const bool o = queryStates->inSetOut(query_toid, search_depth);
				const bool i = queryStates->inSetIn(query_toid, search_depth);
				const bool b = (o && i);
				//const auto label = queryGraph[query_toid].label();
				const auto label = queryGraphPtr->GetLabel(query_toid);
				if (b) {
					bothNewCount[label]--; 
					outNewCount[label]--; 
					inNewCount[label]--;
				}
				else if (o) {outNewCount[label]--;}
				else if (i) {inNewCount[label]--;}
				else {notNewCount[label]--;}
			}
			else {
				const auto target_toid = (notMapped) ? target_id : mapping[query_toid];
				if (targetGraphPtr->ExistEdge(target_id, target_toid, tempEdge.label()) == false)return false;
			}

		}
		if (checkNewCount() == false) {
		//	cnc++;
			return false;
		}
		clearNewCount();

		//for (const auto& tempEdge : target_node.inEdges()) {
		for (const auto& tempEdge : targetGraphPtr->GetInEdges(target_id)) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = inSetUnmap(target_fromid);
			if (notMapped && target_id != target_fromid) {
				const bool o = inSetOut(target_fromid);
				const bool i = inSetIn(target_fromid);
				const bool b = (o && i);
				//const auto label = targetGraph[target_fromid].label();
				const auto label = targetGraphPtr->GetLabel(target_fromid);
				if (b) {
					bothNewCount[label]++;
					outNewCount[label]++;
					inNewCount[label]++;

				}
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
			}

		}

		//for (const auto& tempEdge : query_node.inEdges()) {
		for (const auto& tempEdge : queryGraphPtr->GetInEdges(query_id)) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = queryStates->inSetUnmap(query_fromid,search_depth);
			if (notMapped && query_id != query_fromid) {
				const bool o = queryStates->inSetOut(query_fromid,search_depth);
				const bool i = queryStates->inSetIn(query_fromid,search_depth);
				const bool b = (o && i);
				//const auto label = queryGraph[query_fromid].label();
				const auto label = queryGraphPtr->GetLabel(query_fromid);
				if (b) {
					bothNewCount[label]--;
					outNewCount[label]--;
					inNewCount[label]--;
				}
				else if (o) { outNewCount[label]--; }
				else if (i) { inNewCount[label]--; }
				else { notNewCount[label]--; }
			}
			else {
				const auto target_fromid = (notMapped) ? target_id : mapping[query_fromid];
				if (targetGraphPtr->ExistEdge(target_fromid, target_id, tempEdge.label()) == false)return false;
			}
		}
		if (checkNewCount() == false) {
	//		cnc++;
			return false;
		}
		return true;
	}

	inline bool inSetUnmap(NodeIDType target_id)const { return mappingAux[target_id] == NO_MAP; }
	inline bool inSetIn(NodeIDType target_id)const { return inSetUnmap(target_id) && in_depth[target_id]; }
	inline bool inSetOut(NodeIDType target_id)const { return inSetUnmap(target_id) && out_depth[target_id]; }

	inline bool simpleAddAble(const NodeIDType& query_id, const NodeIDType& target_id)const {
		if (mappingAux[target_id] != NO_MAP)return false;
		if (queryGraphPtr->GetLabel(query_id) != targetGraphPtr->GetLabel(target_id)) return false;
		if (queryGraphPtr->GetInDegree(query_id) > targetGraphPtr->GetInDegree(target_id)) return false;
		if (queryGraphPtr->GetOutDegree(query_id) > targetGraphPtr->GetOutDegree(target_id)) return false;
		//if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(target_id)) == false || (targetGraphPtr->node(target_id) >= queryGraphPtr->node(query_id)) == false)	return false;
		return true;
	}

public:
	State(const GraphType& _q, const GraphType& _t, shared_ptr<const SubgraphMatchStates<EdgeLabel>> _queryStates) :queryGraphPtr(&_q), targetGraphPtr(&_t),
		queryStates(_queryStates), in_depth(_t.Size()), out_depth(_t.Size())
	{
		const auto queryGraphSize = _q.Size();
		const auto targetGraphSize = _t.Size();

		mappingAux.resize(targetGraphSize, NO_MAP);
		mapping.resize(queryGraphSize, NO_MAP);
		//const auto labelNum = max(_q.maxLabel(), _t.maxLabel()) + 1;
		uint32_t labelNum = 0;
		for (const auto &lab:_q.GetLabels()){
			labelNum = max(labelNum,lab);
		}
		for (const auto &lab:_t.GetLabels()){
			labelNum = max(labelNum,lab);
		}
		labelNum++;


		inNewCount.resize(labelNum);
		outNewCount.resize(labelNum);
		bothNewCount.resize(labelNum);
		notNewCount.resize(labelNum);

	};
	State() = default;

	void calCandidatePairs(const NodeIDType query_id, Tasks<EdgeLabel>& container)const {
		if (queryStates->inSetIn(query_id, search_depth)) {
			auto target_pre_index = queryStates->inDepth(query_id, search_depth);
			auto target_pre_id = mapping[queryStates->matchID(target_pre_index - 1)];
			//const auto& edges = targetGraphPtr->node(target_pre_id).inEdges();
			const auto& edges = targetGraphPtr->GetInEdges(target_pre_id);
			container.giveTasks(edges.size(), edges.data(), TASK_TYPE::S);
		}
		else if (queryStates->inSetOut(query_id, search_depth)) {
			auto target_pre_index = queryStates->outDepth(query_id, search_depth);
			auto target_pre_id = mapping[queryStates->matchID(target_pre_index - 1)];
			//const auto& edges = targetGraphPtr->node(target_pre_id).outEdges();
			const auto& edges = targetGraphPtr->GetOutEdges(target_pre_id);
			container.giveTasks(edges.size(), edges.data(), TASK_TYPE::T);
		}
		else {
			container.giveTasks(targetGraphPtr->Size());
		}
	}
	bool AddAble(const NodeIDType& query_id, const NodeIDType& target_id)
	{
		if (simpleAddAble(query_id, target_id) == false) return false;
	/*	if (mappingAux[target_id] != NO_MAP) return false;
		if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(target_id)) == false || (targetGraphPtr->node(target_id) >= queryGraphPtr->node(query_id)) == false) return false;*/
#ifdef INDUCE_ISO
		const bool answer = induceCheck(query_id, target_id);
#elif defined(NORMAL_ISO)
		const bool answer = normalCheck(query_id, target_id);
#endif
		return answer;
	}
	void AddPair(const NodeIDType& query_id, const NodeIDType& target_id) {
		mapping[query_id] = target_id;
		mappingAux[target_id] = query_id;
		search_depth++;

		const auto id = target_id;
		
		const GraphType& graph = *targetGraphPtr;
		//const auto& node = graph.node(id);
		for (const auto& tempEdge : targetGraphPtr->GetInEdges(target_id)) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped and not in set in
			if (!inSetIn(nodeID) && inSetUnmap(nodeID)) {
				in_depth[nodeID] = search_depth;
			}
		}
		//for (const auto& tempEdge : node.outEdges()) {
		for (const auto& tempEdge : targetGraphPtr->GetOutEdges(target_id)) {
			const auto& nodeID = tempEdge.target();
			if (!inSetOut(nodeID) && inSetUnmap(nodeID)) {
				out_depth[nodeID] = search_depth;
			}
		}
	}
	void RemovePair(const NodeIDType queryNodeID)  //query node id
	{
		//NodeIDType& targetNodeID = mapping[queryNodeID];
		//const auto id = targetNodeID;
		//mappingAux[targetNodeID] = NO_MAP;
		//targetNodeID = NO_MAP;
		const auto targetNodeID = mapping[queryNodeID];
		mapping[queryNodeID] = NO_MAP;
		mappingAux[targetNodeID] = NO_MAP;

		const GraphType& graph = *targetGraphPtr;
		//const auto& node = graph.node(id);

		//for (const auto& tempEdge : node.inEdges()) {
		for (const auto& tempEdge : targetGraphPtr->GetInEdges(targetNodeID)) {
			const auto& nodeID = tempEdge.source();
			auto& nodeDepth = in_depth[nodeID];
			if (nodeDepth == search_depth)	nodeDepth = 0;

		}
		for (const auto& tempEdge :targetGraphPtr->GetOutEdges(targetNodeID)) {
			const auto& nodeID = tempEdge.target();
			auto& nodeDepth = out_depth[nodeID];
			if (nodeDepth == search_depth)	nodeDepth = 0;
		}
		search_depth--;
	}
	inline bool isCoverQueryGraph()const { return (queryGraphPtr->Size() == search_depth); }
	MapType GetMapping(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) cout << "WARNING : Map is not covering the whole quert graph\n";
		return mapping;
	}
	size_t depth() const { return search_depth; }


};


template<typename EdgeLabel>
class StateL {
	using GraphType = GraphS<EdgeLabel>;

private:
	shared_ptr<const SubgraphMatchStates<EdgeLabel>> queryStates;
	const GraphType* targetGraphPtr, * queryGraphPtr;
	size_t search_depth = 0;
	MapType mapping;
	MapType mappingAux; //from target to query

	//check the mapping is still consistent after add this pair

	bool induceCheck(const NodeIDType& query_id, const NodeIDType& target_id) {
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;

		const auto& query_node = queryGraph.node(query_id);
		const auto& target_node = targetGraph.node(target_id);

		int out_edge_inmap_num = 0, in_edge_inmap_num = 0;
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map

		for (const auto& tempEdge : target_node.outEdges()) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = inSetUnmap(target_toid);
			if (notMapped && target_toid != target_id) {
				continue;
			}
			else {
				++out_edge_inmap_num;
				const auto query_toid = (notMapped) ? query_id : mappingAux[target_toid];
				if (queryGraph.existEdge(query_id, query_toid, tempEdge.label()) == false) return false;
			}
		}

		for (const auto& tempEdge : query_node.outEdges()) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = queryStates->inSetUnmap(query_toid, search_depth);
			if (notMapped && query_toid != query_id) {
				continue;
			}
			else --out_edge_inmap_num;

		}
		if (out_edge_inmap_num) return false;
		for (const auto& tempEdge : target_node.inEdges()) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = inSetUnmap(target_fromid);
			if (notMapped && target_id != target_fromid) {
				continue;
			}
			else {
				++in_edge_inmap_num;
				const auto query_fromid = (notMapped) ? query_id : mappingAux[target_fromid];
				if (queryGraph.existEdge(query_fromid, query_id, tempEdge.label()) == false)return false;
			}
		}

		for (const auto& tempEdge : query_node.inEdges()) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = queryStates->inSetUnmap(query_fromid, search_depth);
			if (notMapped && query_id != query_fromid) {
				continue;
			}
			else --in_edge_inmap_num;	
		}
		if (in_edge_inmap_num)return false;
		return true;
	}


	bool inSetUnmap(NodeIDType target_id)const { return mappingAux[target_id] == NO_MAP; }

	inline bool simpleAddAble(const NodeIDType& query_id, const NodeIDType& target_id)const {
		if (mappingAux[target_id] != NO_MAP)return false;
		if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(target_id)) == false || (targetGraphPtr->node(target_id) >= queryGraphPtr->node(query_id)) == false)	return false;
		return true;
	}

public:
	StateL(const GraphType& _q, const GraphType& _t, shared_ptr<const SubgraphMatchStates<GraphType>> _queryStates) :queryGraphPtr(&_q), targetGraphPtr(&_t),
		queryStates(_queryStates)
	{
		const auto queryGraphSize = _q.size();
		const auto targetGraphSize = _t.size();

		mappingAux.resize(targetGraphSize, NO_MAP);
		mapping.resize(queryGraphSize, NO_MAP);
		const auto labelNum = max(_q.maxLabel(), _t.maxLabel()) + 1;
	};
	StateL() = default;

	void calCandidatePairs(const NodeIDType query_id, Tasks<EdgeLabel>& container)const {
		if (queryStates->inSetIn(query_id, search_depth)) {
			auto target_pre_index = queryStates->inDepth(query_id, search_depth);
			auto target_pre_id = mapping[queryStates->matchID(target_pre_index - 1)];
			const auto& edges = targetGraphPtr->node(target_pre_id).inEdges();
			container.giveTasks(edges.size(), edges.data(), TASK_TYPE::S);
		}
		else if (queryStates->inSetOut(query_id, search_depth)) {
			auto target_pre_index = queryStates->outDepth(query_id, search_depth);
			auto target_pre_id = mapping[queryStates->matchID(target_pre_index - 1)];
			const auto& edges = targetGraphPtr->node(target_pre_id).outEdges();
			container.giveTasks(edges.size(), edges.data(), TASK_TYPE::T);
		}
		else {
			container.giveTasks(targetGraphPtr->Size());
		}
	}
	bool AddAble(const NodeIDType& query_id, const NodeIDType& target_id)
	{
		if (simpleAddAble(query_id, target_id) == false) return false;
#ifdef INDUCE_ISO
		const bool answer = induceCheck(query_id, target_id);
#elif defined(NORMAL_ISO)
		const bool answer = normalCheck(query_id, target_id);
#endif
		return answer;
	}
	void AddPair(const NodeIDType& query_id, const NodeIDType& target_id) {
		mapping[query_id] = target_id;
		mappingAux[target_id] = query_id;
		search_depth++;
	}
	void RemovePair(const NodeIDType queryNodeID)  //query node id
	{
		const NodeIDType targetNodeID = mapping[queryNodeID];
		mapping[queryNodeID] = NO_MAP;
		mappingAux[targetNodeID] = NO_MAP;

		search_depth--;
	}
	inline bool isCoverQueryGraph()const { return (queryGraphPtr->Size() == search_depth); }
	MapType GetMapping(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) cout << "WARNING : Map is not covering the whole quert graph\n";
		return mapping;
	}
	size_t depth() const { return search_depth; }
};

template<class EdgeType>
shared_ptr<const SubgraphMatchStates<EdgeType>> makeSubgraphState(const GraphS<EdgeType>& g, const vector<NodeIDType>& ms) {
	assert(g.Size() == ms.size());
	vector<size_t> match_place(ms.size());
	for (int i = 0; i < ms.size(); ++i) {
		(match_place)[ms[i]] = i + 1;
	}
	auto ptr = new SubgraphMatchStates<EdgeType>(g, match_place,ms);
	shared_ptr<const SubgraphMatchStates<EdgeType>> p(ptr);

	LOOP(i, 1, ms.size()) {
		ptr->addNode(ms[i - 1], i);
	}
	return p;
}

}
