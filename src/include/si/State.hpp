#pragma once
#include<vector>
#include<time.h>
#include"graph/Graph.hpp"
#include"NodeSet.hpp"
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

class GraphStateBase {
protected:
	size_t searchDepth = 0;
	vector<size_t> in_ref_times, out_ref_times;
public:
	GraphStateBase() = default;
	GraphStateBase(const size_t& graph_size) :
		in_ref_times(graph_size), out_ref_times(graph_size)
	{
	}
	inline size_t inRefTimes(const NodeIDType id)const { return in_ref_times[id]; }
	inline size_t outRefTimes(const NodeIDType id)const { return out_ref_times[id]; }

};
template<class GraphType>
class TargetGraphMatchState :public GraphStateBase {
	NodeSetWithDepth unmap, in, out;
	const GraphType* graphPointer = nullptr;
	vector<size_t> in_depth, out_depth;
public:
	TargetGraphMatchState() = default;
	TargetGraphMatchState(const GraphType& g, const size_t maxDepth = 0) :GraphStateBase(g.size()), graphPointer(&g), in(g.size(), maxDepth), out(g.size(), maxDepth), unmap(g.size(), maxDepth),
		in_depth(g.size()), out_depth(g.size())
	{
		for (const auto& node : g.nodes()) unmap.insert(node.id(), 0);
	}
	void addNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		in.erase(id, in_depth[id]);
		out.erase(id, out_depth[id]);
		unmap.erase(id, 0);
		searchDepth++;
		in.prepare(searchDepth);
		out.prepare(searchDepth);
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool i = inSetIn(nodeID);
			const bool n = (i) ? true : inSetUnmap(nodeID);
			if (!n)continue;
			if (!i) {
				in.insert(nodeID, searchDepth);
				in_depth[nodeID] = searchDepth;
			}
			++in_ref_times[nodeID];
		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = inSetOut(nodeID);
			const bool n = (o) ? true : inSetUnmap(nodeID);
			if (!n)continue;
			if (!o) {
				out.insert(nodeID, searchDepth);
				out_depth[nodeID] = searchDepth;
			}
			++out_ref_times[nodeID];
		}
		return;
	}
	void deleteNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		const auto& node = graph.node(id);

		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = inSetUnmap(nodeID);
			if (!n)continue;
			in_ref_times[nodeID]--;
			auto& nodeDepth = in_depth[nodeID];
			if (nodeDepth == searchDepth)	nodeDepth = 0;

		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = inSetUnmap(nodeID);
			if (!n)continue;
			out_ref_times[nodeID]--;
			auto& nodeDepth = out_depth[nodeID];
			if (nodeDepth == searchDepth)	nodeDepth = 0;
		}
		in.pop(searchDepth);
		out.pop(searchDepth);
		--searchDepth;
		if (in_depth[id]) 	in.insert(id, in_depth[id]);
		if (out_depth[id]) 	out.insert(id, out_depth[id]);
		unmap.insert(id, 0);
	}
	inline size_t inDepth(const NodeIDType id)const { return in_depth[id]; }
	inline size_t outDepth(const NodeIDType id)const { return out_depth[id]; }
	inline bool inSetIn(const NodeIDType id)const { return in.exist(id); }
	inline bool inSetOut(const NodeIDType id)const { return out.exist(id); }
	inline bool inSetUnmap(const NodeIDType id)const { return unmap.exist(id); }
	inline void getInSet(const size_t depth, const NodeIDType*& begin, const NodeIDType*& end)const { in.getSet(depth, begin, end); }
	inline void getOutSet(const size_t depth, const NodeIDType*& begin, const NodeIDType*& end)const { out.getSet(depth, begin, end); }
	inline void getUnmapSet(const NodeIDType*& begin, const NodeIDType*& end)const { unmap.getSet(0, begin, end); }
};

template<typename GraphType>
class SubgraphMatchState :public GraphStateBase {
	NodeSetSimple unmap, in, out;
	const GraphType* graphPointer;
	shared_ptr<vector<size_t>> in_depth, out_depth;
public:
	SubgraphMatchState() = default;
	inline size_t inDepth(const NodeIDType id)const { return (*in_depth)[id]; }
	inline size_t outDepth(const NodeIDType id)const { return (*out_depth)[id]; }
	SubgraphMatchState(const GraphType& g, const size_t maxDepth = 0) :GraphStateBase(g.size()), graphPointer(&g), in(g.size()), out(g.size()), unmap(g.size()),
		in_depth(new vector<size_t>(g.size())), out_depth(new vector<size_t>(g.size()))
	{
		for (const auto& node : g.nodes()) unmap.insert(node.id());
	}
	void addNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		unmap.erase(id);
		searchDepth++;
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool i = inSetIn(nodeID);
			const bool n = (i) ? true : inSetUnmap(nodeID);
			if (!n)continue;
			if (!i)	(*in_depth)[nodeID] = searchDepth;
			++in_ref_times[nodeID];
		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = inSetOut(nodeID);
			const bool n = (o) ? true : inSetUnmap(nodeID);
			if (!n)continue;
			if (!o) (*out_depth)[nodeID] = searchDepth;
			++out_ref_times[nodeID];
		}
		return;
	}
	inline bool inSetIn(const NodeIDType id)const { return unmap.exist(id) && inDepth(id) && inDepth(id) <= searchDepth; }
	inline bool inSetOut(const NodeIDType id)const { return unmap.exist(id) && outDepth(id) && outDepth(id) <= searchDepth; }
	inline bool inSetUnmap(const NodeIDType id)const { return unmap.exist(id); }
};

template<typename GraphType>
class State {
	typedef typename GraphType::NodeType NodeType;

	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef typename NodeSetWithLabel<GraphType>::Nodes NodeSetWithLabelUnit;
private:
	TargetGraphMatchState<GraphType> targetState;
	shared_ptr<SubgraphMatchState<GraphType>[]> queryStates;
	const GraphType* targetGraphPtr, * queryGraphPtr;
	size_t searchDepth = 0;
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

		clearNewCount();
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map

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
				}
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
			}
			else {
				const auto query_toid = (notMapped) ? query_id : mappingAux[target_toid];
				if (queryGraph.existEdge(query_id, query_toid, tempEdge.label()) == false) return false;
			}
		}

		for (const auto& tempEdge : query_node.outEdges()) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = queryStates[searchDepth].inSetUnmap(query_toid);
			if (notMapped && query_toid != query_id) {
				const bool o = queryStates[searchDepth].inSetOut(query_toid);
				const bool i = queryStates[searchDepth].inSetIn(query_toid);
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

				}
				else if (o)	outNewCount[label]++;
				else if (i) inNewCount[label]++;

				else ++notNewCount[label];
			}
			else {
				const auto query_fromid = (notMapped) ? query_id : mappingAux[target_fromid];
				if (queryGraph.existEdge(query_fromid, query_id, tempEdge.label()) == false)return false;
			}
		}

		for (const auto& tempEdge : query_node.inEdges()) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = queryStates[searchDepth].inSetUnmap(query_fromid);
			if (notMapped && query_id != query_fromid) {
				const bool o = queryStates[searchDepth].inSetOut(query_fromid);
				const bool i = queryStates[searchDepth].inSetIn(query_fromid);
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
				const auto target_fromid = (notMapped) ? target_id : mapping[query_fromid];
				if (targetGraph.existEdge(target_fromid, target_id, tempEdge.label()) == false)return false;
			}
		}
		return true;
	}
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
			const bool notMapped = queryStates[searchDepth].inSetUnmap(query_toid);
			if (notMapped && query_toid != query_id) {
				const bool o = queryStates[searchDepth].inSetOut(query_toid);
				const bool i = queryStates[searchDepth].inSetIn(query_toid);
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
			const bool notMapped = queryStates[searchDepth].inSetUnmap(query_fromid);
			if (notMapped && query_id != query_fromid) {
				const bool o = queryStates[searchDepth].inSetOut(query_fromid);
				const bool i = queryStates[searchDepth].inSetIn(query_fromid);
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

public:
	State(const GraphType& _q, const GraphType& _t, shared_ptr<SubgraphMatchState<GraphType>[]> _queryStates) :queryGraphPtr(&_q), targetGraphPtr(&_t), targetState(_t, _q.size()), queryStates(_queryStates)
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
	};

	State() = default;

public:
	void calCandidatePairs(const NodeIDType query_id, const NodeIDType*& begin, const NodeIDType*& end)const
	{
#if defined(INDUCE_ISO)
		if (queryStates[searchDepth].inSetIn(query_id))	targetState.getInSet(queryStates[searchDepth].inDepth(query_id), begin, end);
		else if (queryStates[searchDepth].inSetOut(query_id))	targetState.getOutSet(queryStates[searchDepth].outDepth(query_id), begin, end);
		else targetState.getUnmapSet(begin, end);
#endif

#ifdef NORMAL_ISO
		targetState.getUnmapSet(begin, end);
#endif

	}

	bool checkPair(const NodeIDType& query_id, const NodeIDType& target_id)
	{
		if (queryGraphPtr->node(query_id).isSameType(targetGraphPtr->node(target_id)) == false || (targetGraphPtr->node(target_id) >= queryGraphPtr->node(query_id)) == false) return false;
#ifdef INDUCE_ISO
		if (queryStates[searchDepth].inRefTimes(query_id) != targetState.inRefTimes(target_id)) return false;
		if (queryStates[searchDepth].outRefTimes(query_id) != targetState.outRefTimes(target_id)) return false;
		if (queryStates[searchDepth].inDepth(query_id) != targetState.inDepth(target_id))return false;
		if (queryStates[searchDepth].outDepth(query_id) != targetState.outDepth(target_id))return false;

		const bool answer = induceCheck(query_id, target_id);
#elif defined(NORMAL_ISO)
		if (queryStates[searchDepth].inRefTimes(query_id) > targetState.inRefTimes(target_id)) return false;
		if (queryStates[searchDepth].outRefTimes(query_id) > targetState.outRefTimes(target_id)) return false;
		if (queryStates[searchDepth].inDepth(query_id) < targetState.inDepth(target_id))return false;
		if (queryStates[searchDepth].outDepth(query_id) < targetState.outDepth(target_id))return false;
		const bool answer = normalCheck(query_id, target_id));
#endif
		return answer;
	}

	void pushPair(const NodeIDType& query_id, const NodeIDType& target_id) {
		mapping[query_id] = target_id;
		mappingAux[target_id] = query_id;
		targetState.addNode(target_id);
		searchDepth++;
}
	void popPair(const NodeIDType queryNodeID)  //query node id
	{
		NodeIDType& targetNodeID = mapping[queryNodeID];
		targetState.deleteNode(targetNodeID);
		mappingAux[targetNodeID] = NO_MAP;
		targetNodeID = NO_MAP;
		searchDepth--;
	}
	inline bool isCoverQueryGraph()const {
		return (queryGraphPtr->size() == searchDepth);
	}
	MapType getMap(bool showNotCoverWarning = true) const {
		if (isCoverQueryGraph() == false && showNotCoverWarning) {
			cout << "WARNING : Map is not covering the whole quert graph\n";
		}
		return mapping;
	}
	size_t depth() const { return searchDepth; }


};
template<class GraphType>
shared_ptr<SubgraphMatchState<GraphType>[]> makeSubgraphState(const GraphType& g, const vector<NodeIDType>& ms) {
	assert(g.size() == ms.size());
	//the final state will never be used , so this function will not generate the final state;
	auto ptr = new SubgraphMatchState<GraphType>[ms.size()];
	shared_ptr<SubgraphMatchState<GraphType>[]> p(ptr);
	p[0] = move(SubgraphMatchState<GraphType>(g));
	LOOP(i, 1, ms.size()) {
		ptr[i] = ptr[i - 1];
		ptr[i].addNode(ms[i - 1]);
	}
	return p;
}

}