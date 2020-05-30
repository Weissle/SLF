#pragma once
#include<vector>
#include<time.h>
#include"graph/Graph.hpp"
#include"graph/Node.hpp"
#include<assert.h>
#include<iostream>
#include<algorithm>
#include<memory>
#include"common.h"
#include<si/si_marcos.h>
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

template<typename GraphType>
class SubgraphMatchStates {
	//	size_t search_depth = 0;
	const GraphType* graphPointer;
	vector<size_t> in_depth, out_depth;
	const vector<size_t> match_depth;
public:
	SubgraphMatchStates() = default;
	SubgraphMatchStates(const GraphType& g, const vector<size_t>& _match_depth) :graphPointer(&g), match_depth(_match_depth),
		in_depth(g.size()), out_depth(g.size())
	{}
	void addNode(const NodeIDType id, const size_t search_depth) {
		const GraphType& graph = *graphPointer;
		//		search_depth++;
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped and not in set in
			if (!inSetIn(nodeID, search_depth) && inSetUnmap(nodeID, search_depth)) 	in_depth[nodeID] = search_depth;
		}
		for (const auto& tempEdge : node.outEdges()) {
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
	inline bool inSetUnmap(const NodeIDType id, const size_t search_depth)const { return match_depth[id] >= search_depth; }
};

template<typename GraphType>
class State {
	typedef typename GraphType::NodeType NodeType;

	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

private:
	vector<size_t> in_depth, out_depth;
	vector<NodeIDType> match_sequence;

	shared_ptr<const SubgraphMatchStates<GraphType>> queryStates;
	const GraphType* targetGraphPtr, * queryGraphPtr;
	size_t search_depth = 0;
	MapType mapping;
	MapType mappingAux; //from target to query

	size_t labelNum;

	vector<size_t> inNewCount, outNewCount, bothNewCount, notNewCount;
	//used in look forward 2 
	void clearNewCount() {
		const auto temp = sizeof(size_t) * labelNum;
		memset(inNewCount.data(), 0, temp);
		memset(outNewCount.data(), 0, temp);
		memset(bothNewCount.data(), 0, temp);
#ifdef INDUCE_ISO
		memset(notNewCount.data(), 0, temp);
#endif
	}
	//check the mapping is still consistent after add this pair

	bool induceCheck(const NodeIDType& query_id, const NodeIDType& target_id) {
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;

		const auto& query_node = queryGraph.node(query_id);
		const auto& target_node = targetGraph.node(target_id);

		int out_edge_inmap_num = 0, in_edge_inmap_num = 0;
		clearNewCount();
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map

		for (const auto& tempEdge : target_node.outEdges()) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = inSetUnmap(target_toid);
			if (notMapped && target_toid != target_id) {
				const bool o = inSetOut(target_toid);
				const bool i = inSetIn(target_toid);
				const bool b = (i && o);
				const auto label = targetGraph[target_toid].label();
				if (b) {
					bothNewCount[label]++;
				}
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
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
				const bool o = queryStates->inSetOut(query_toid, search_depth);
				const bool i = queryStates->inSetIn(query_toid, search_depth);
				const bool b = (o && i);
				const auto label = queryGraph[query_toid].label();
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
				else {
					if (notNewCount[label]--);
					else return false;
				}
			}
			else {
				--out_edge_inmap_num;
			}

		}
		if (out_edge_inmap_num) return false;
		clearNewCount();

		for (const auto& tempEdge : target_node.inEdges()) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = inSetUnmap(target_fromid);
			if (notMapped && target_id != target_fromid) {
				const bool o = inSetOut(target_fromid);
				const bool i = inSetIn(target_fromid);
				const bool b = (o && i);
				const auto label = targetGraph[target_fromid].label();
				if (b) 	bothNewCount[label]++;
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
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
				const bool o = queryStates->inSetOut(query_fromid, search_depth);
				const bool i = queryStates->inSetIn(query_fromid, search_depth);
				const bool b = (o && i);
				const auto label = queryGraph[query_fromid].label();
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
				else {
					if (notNewCount[label]--);
					else return false;
				}
			}
			else {
				--in_edge_inmap_num;
			}
		}
		if (in_edge_inmap_num)return false;
		return true;
	}
/*	
	bool normalCheck(const NodeIDType& query_id, const NodeIDType& target_id) {
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;

		const auto& query_node = queryGraph.node(query_id);
		const auto& target_node = targetGraph.node(target_id);

		clearNewCount();

		for (const auto& tempEdge : target_node.outEdges()) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = targetState.inSetUnmap(target_toid);
			if (notMapped && target_toid != target_id) {
				const bool o = targetState.inSetOut(target_toid);
				const bool i = targetState.inSetIn(target_toid);
				const bool b = (i && o);
				const auto label = targetGraph[target_toid].label();
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

		for (const auto& tempEdge : query_node.outEdges()) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = queryStates->inSetUnmap(query_toid);
			if (notMapped && query_toid != query_id) {
				const bool o = queryStates->inSetOut(query_toid);
				const bool i = queryStates->inSetIn(query_toid);
				const bool b = (o && i);
				const auto label = queryGraph[query_toid].label();
				if (b) {
					if (bothNewCount[label]-- && outNewCount[label]-- && inNewCount[label]--);
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
				else {
					if (notNewCount[label]--);
					else return false;
				}
			}
			else {
				const auto target_toid = (notMapped) ? target_id : mapping[query_toid];
				if (targetGraph.existEdge(target_id, target_toid, tempEdge.label()) == false)return false;
			}

		}

		clearNewCount();

		for (const auto& tempEdge : target_node.inEdges()) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = targetState.inSetUnmap(target_fromid);
			if (notMapped && target_id != target_fromid) {
				const bool o = targetState.inSetOut(target_fromid);
				const bool i = targetState.inSetIn(target_fromid);
				const bool b = (o && i);
				const auto label = targetGraph[target_fromid].label();
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

		for (const auto& tempEdge : query_node.inEdges()) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = queryStates->inSetUnmap(query_fromid);
			if (notMapped && query_id != query_fromid) {
				const bool o = queryStates->inSetOut(query_fromid);
				const bool i = queryStates->inSetIn(query_fromid);
				const bool b = (o && i);
				const auto label = queryGraph[query_fromid].label();
				if (b) {
					if (bothNewCount[label]-- && outNewCount[label]-- && inNewCount[label]--);
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
				else {
					if (notNewCount[label]--);
					else return false;
				}
			}
			else {
				const auto target_fromid = (notMapped) ? target_id : mapping[query_fromid];
				if (targetGraph.existEdge(target_fromid, target_id, tempEdge.label()) == false)return false;
			}
		}
		return true;
	}
*/
	bool inSetUnmap(NodeIDType target_id)const { return mappingAux[target_id] == NO_MAP; }
	bool inSetIn(NodeIDType target_id)const { return inSetUnmap(target_id) && in_depth[target_id]; }
	bool inSetOut(NodeIDType target_id)const { return inSetUnmap(target_id) && out_depth[target_id]; }


public:
	State(const GraphType& _q, const GraphType& _t, shared_ptr<const SubgraphMatchStates<GraphType>> _queryStates) :queryGraphPtr(&_q), targetGraphPtr(&_t), 
		queryStates(_queryStates), in_depth(_t.size()), out_depth(_t.size())
	{
		const auto queryGraphSize = _q.size();
		const auto targetGraphSize = _t.size();

		mappingAux.resize(targetGraphSize, NO_MAP);
		mapping.resize(queryGraphSize, NO_MAP);
		labelNum = max(_q.maxLabel(), _t.maxLabel()) + 1;

		inNewCount.resize(labelNum);
		outNewCount.resize(labelNum);
		bothNewCount.resize(labelNum);
		notNewCount.resize(labelNum);

		match_sequence.reserve(_q.size()+1);
		match_sequence.push_back(NO_MAP);
	};
	State() = default;

	void calCandidatePairs(const NodeIDType query_id, vector<NodeIDType> &container)const
	{
#if defined(INDUCE_ISO)

		if (queryStates->inSetIn(query_id, search_depth)) {
			auto target_seq_index = queryStates->inDepth(query_id,search_depth);
			auto target_pre_id = match_sequence[target_seq_index];
			for (const auto& e : targetGraphPtr->node(target_pre_id).inEdges()) {
				auto temp_id = e.source();
				if (mappingAux[temp_id] != NO_MAP)continue;
				else if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(temp_id)) == false || (targetGraphPtr->node(temp_id) >= queryGraphPtr->node(query_id)) == false)	continue;
				else container.push_back(temp_id);
			}
		}
		else if (queryStates->inSetOut(query_id, search_depth)) {
			auto target_seq_index = queryStates->outDepth(query_id,search_depth);
			auto target_pre_id = match_sequence[target_seq_index];
			for (const auto& e : targetGraphPtr->node(target_pre_id).outEdges()) {
				auto temp_id = e.target();
				if (mappingAux[temp_id] != NO_MAP)continue;
				else if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(temp_id)) == false || (targetGraphPtr->node(temp_id) >= queryGraphPtr->node(query_id)) == false)	continue;
				else container.push_back(temp_id);
			}
		}
		else {
			for (NodeIDType id = 0; id < targetGraphPtr->size(); ++id) {
				if (mappingAux[id] != NO_MAP) continue;
				else if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(id)) == false || (targetGraphPtr->node(id) >= queryGraphPtr->node(query_id)) == false)	continue;
				else container.push_back(id);

			}
		}
#endif

#ifdef NORMAL_ISO
		for (NodeIDType id = 0; id < targetGraphPtr->size(); ++id) {
			if (mappingAux[id] != NO_MAP) continue;
			else if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(id)) == false || (targetGraphPtr->node(temp_id) >= queryGraphPtr->node(query_id)) == false)	continue;
			else container.push_back(id);
		}
#endif
	}

	bool checkPair(const NodeIDType& query_id, const NodeIDType& target_id)
	{
	//	if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(target_id)) == false || (targetGraphPtr->node(target_id) >= queryGraphPtr->node(query_id)) == false) return false;
#ifdef INDUCE_ISO
/*		if (queryStates->inDepth(query_id, search_depth) != in_depth[target_id])return false;
		if (queryStates->outDepth(query_id, search_depth) != out_depth[target_id])return false;
		*/
		const bool answer = induceCheck(query_id, target_id);
#elif defined(NORMAL_ISO)
		if (queryStates->inDepth(query_id, search_depth) < in_depth[target_id])return false;
		if (queryStates->outDepth(query_id, search_depth) < out_depth[target_id])return false;
		const bool answer = normalCheck(query_id, target_id));
#endif
		return answer;
	}
	void pushPair(const NodeIDType& query_id, const NodeIDType& target_id) {
		mapping[query_id] = target_id;
		mappingAux[target_id] = query_id;
	//	targetState.addNode(target_id);
		search_depth++;

		const auto id = target_id;
		match_sequence.push_back(id);
		
		const GraphType& graph = *targetGraphPtr;
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped and not in set in
			if (!inSetIn(nodeID) && inSetUnmap(nodeID)) {
				in_depth[nodeID] = search_depth;
			}
		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			if (!inSetOut(nodeID) && inSetUnmap(nodeID)) {
				out_depth[nodeID] = search_depth;
			}
		}

		


	}
	void popPair(const NodeIDType queryNodeID)  //query node id
	{
		NodeIDType& targetNodeID = mapping[queryNodeID];
	//	targetState.deleteNode(targetNodeID);
		mappingAux[targetNodeID] = NO_MAP;
		targetNodeID = NO_MAP;

		const auto id = match_sequence.back();
		match_sequence.pop_back();
		const GraphType& graph = *targetGraphPtr;
		const auto& node = graph.node(id);

		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			auto& nodeDepth = in_depth[nodeID];
			if (nodeDepth == search_depth)	nodeDepth = 0;

		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			auto& nodeDepth = out_depth[nodeID];
			if (nodeDepth == search_depth)	nodeDepth = 0;
		}
		search_depth--;
	}
	inline bool isCoverQueryGraph()const {
		return (queryGraphPtr->size() == search_depth);
	}
	MapType getMap(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
	size_t depth() const { return search_depth; }


};
template<class GraphType>
shared_ptr<const SubgraphMatchStates<GraphType>> makeSubgraphState(const GraphType& g, shared_ptr<const vector<NodeIDType>> msp) {
	assert(g.size() == msp->size());
	vector<size_t> match_place(msp->size());
	for (int i = 0; i < msp->size(); ++i) {
		(match_place)[(*msp)[i]] = i;
	}
	auto ptr = new SubgraphMatchStates<GraphType>(g, match_place);
	shared_ptr<const SubgraphMatchStates<GraphType>> p(ptr);

	LOOP(i, 1, msp->size()) {
		ptr->addNode((*msp)[i - 1], i);
	}
	return p;
}

}