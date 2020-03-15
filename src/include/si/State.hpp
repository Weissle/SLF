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

template<typename GraphType>
class State;
template<typename GraphType>
class GraphStateBase {
protected:
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	const GraphType* graphPointer = nullptr;
	size_t searchDepth = 0;
	vector<size_t> inDepth, outDepth;
	vector<size_t> inRefTimes, outRefTimes;
	friend class State<GraphType>;
public:
	GraphStateBase() = default;
	GraphStateBase(const GraphType& g) :graphPointer(&g) {
		size_t graphSize = g.size();
		inDepth.resize(graphSize);
		outDepth.resize(graphSize);

		inRefTimes.resize(graphSize);
		outRefTimes.resize(graphSize);
	}
};
template<typename GraphType>
class TargetGraphMatchState :GraphStateBase<GraphType> {
	typedef NodeSetWithDepth<GraphType> NodeSetType;
	NodeSetType unmap, in, out;
public:
	friend class State<GraphType>;
	TargetGraphMatchState() = default;
	TargetGraphMatchState(const GraphType& g, const size_t maxDepth = 0) :GraphStateBase<GraphType>(g) {

		in = NodeSetType(g, maxDepth);
		out = NodeSetType(g, maxDepth);
		unmap = NodeSetType(g, maxDepth);

		for (const auto& node : g.nodes()) unmap.insert(node.id(), 0);
	}
	void addNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		unmap.erase(id, 0);
		in.erase(id, inDepth[id]);
		out.erase(id, outDepth[id]);
		searchDepth++;
		in.prepare(searchDepth);
		out.prepare(searchDepth);
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool i = IN_NODE_SET(in, nodeID);
			const bool n = (i) ? true : IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			if (!i) {
				in.insert(nodeID, searchDepth);
				inDepth[nodeID] = searchDepth;
			}
			++inRefTimes[nodeID];
		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = IN_NODE_SET(out, nodeID);
			const bool n = (o) ? true : IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			if (!o) {
				out.insert(nodeID, searchDepth);
				outDepth[nodeID] = searchDepth;
			}
			++outRefTimes[nodeID];
		}
		return;
	}
	void deleteNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			const bool n = IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			inRefTimes[nodeID]--;
			auto& nodeDepth = inDepth[nodeID];
			if (nodeDepth == searchDepth)	nodeDepth = 0;

		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool n = IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			outRefTimes[nodeID]--;
			auto& nodeDepth = outDepth[nodeID];
			if (nodeDepth == searchDepth)	nodeDepth = 0;
		}
		in.pop(searchDepth);
		out.pop(searchDepth);
		--searchDepth;
		if (inDepth[id]) 	in.insert(id, inDepth[id]);
		if (outDepth[id]) 	out.insert(id, outDepth[id]);
		unmap.insert(id, 0);
	}
};

template<typename GraphType>
class SubgraphMatchState :GraphStateBase<GraphType> {
	using NodeSetType = NodeSetSimple<GraphType>;
	NodeSetType unmap, in, out;
public:
	friend class State<GraphType>;
	SubgraphMatchState() = default;
	SubgraphMatchState(const GraphType& g, const size_t maxDepth = 0) :GraphStateBase<GraphType>(g) {

		in = NodeSetType(g);
		out = NodeSetType(g);
		unmap = NodeSetType(g);
		for (const auto& node : g.nodes()) unmap.insert(node.id());
	}
	void addNode(NodeIDType id) {
		const GraphType& graph = *graphPointer;
		unmap.erase(id);
		in.erase(id);
		out.erase(id);
		searchDepth++;
		const auto& node = graph.node(id);
		for (const auto& tempEdge : node.inEdges()) {
			const auto& nodeID = tempEdge.source();
			// was not be mapped
			const bool i = IN_NODE_SET(in, nodeID);
			const bool n = (i) ? true : IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			if (!i) {
				in.insert(nodeID);
				inDepth[nodeID] = searchDepth;
			}
			++inRefTimes[nodeID];
		}
		for (const auto& tempEdge : node.outEdges()) {
			const auto& nodeID = tempEdge.target();
			const bool o = IN_NODE_SET(out, nodeID);
			const bool n = (o) ? true : IN_NODE_SET(unmap, nodeID);
			if (!n)continue;
			if (!o) {
				out.insert(nodeID);
				outDepth[nodeID] = searchDepth;
			}
			++outRefTimes[nodeID];
		}
		return;
	}
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

	bool induceCheck(const MapPair& cp) {
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;
		const auto& query_nodeid = cp.first;
		const auto& target_nodeid = cp.second;

		const auto& query_node = queryGraph.node(query_nodeid);
		const auto& target_node = targetGraph.node(target_nodeid);

		clearNewCount();
		// targetSourceNode is the predecessor of three typies of nodes
		//1. not in map AND not self loop  
		//2. self loop and 3. in map

		for (const auto& tempEdge : target_node.outEdges()) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = IN_NODE_SET(targetState.unmap, target_toid);
			if (notMapped && target_toid != target_nodeid) {
				const bool o = IN_NODE_SET(targetState.out, target_toid);
				const bool i = IN_NODE_SET(targetState.in, target_toid);
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
				const auto query_toid = (notMapped) ? query_nodeid : mappingAux[target_toid];
				if (queryGraph.existEdge(query_nodeid, query_toid, tempEdge.label()) == false) return false;
			}
		}

		for (const auto& tempEdge : query_node.outEdges()) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = IN_NODE_SET(queryStates[searchDepth].unmap, query_toid);
			if (notMapped && query_toid != query_nodeid) {
				const bool o = IN_NODE_SET(queryStates[searchDepth].out, query_toid);
				const bool i = IN_NODE_SET(queryStates[searchDepth].in, query_toid);
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
				const auto target_toid = (notMapped) ? target_nodeid : mapping[query_toid];
				if (targetGraph.existEdge(target_nodeid, target_toid, tempEdge.label()) == false)return false;
			}

		}

		clearNewCount();

		for (const auto& tempEdge : target_node.inEdges()) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = IN_NODE_SET(targetState.unmap, target_fromid);
			if (notMapped && target_nodeid != target_fromid) {
				const bool o = IN_NODE_SET(targetState.out, target_fromid);
				const bool i = IN_NODE_SET(targetState.in, target_fromid);
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
				const auto query_fromid = (notMapped) ? query_nodeid : mappingAux[target_fromid];
				if (queryGraph.existEdge(query_fromid, query_nodeid, tempEdge.label()) == false)return false;
			}
		}

		for (const auto& tempEdge : query_node.inEdges()) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = IN_NODE_SET(queryStates[searchDepth].unmap, query_fromid);
			if (notMapped && query_nodeid != query_fromid) {
				const bool o = IN_NODE_SET(queryStates[searchDepth].out, query_fromid);
				const bool i = IN_NODE_SET(queryStates[searchDepth].in, query_fromid);
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
				const auto target_fromid = (notMapped) ? target_nodeid : mapping[query_fromid];
				if (targetGraph.existEdge(target_fromid, target_nodeid, tempEdge.label()) == false)return false;
			}
		}
		return true;
	}
	bool normalCheck(const MapPair& cp) {
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;
		const auto& query_nodeid = cp.first;
		const auto& target_nodeid = cp.second;

		const auto& query_node = queryGraph.node(query_nodeid);
		const auto& target_node = targetGraph.node(target_nodeid);

		clearNewCount();

		for (const auto& tempEdge : target_node.outEdges()) {
			const auto& target_toid = tempEdge.target();
			const bool notMapped = IN_NODE_SET(targetState.unmap, target_toid);
			if (notMapped && target_toid != target_nodeid) {
				const bool o = IN_NODE_SET(targetState.out, target_toid);
				const bool i = IN_NODE_SET(targetState.in, target_toid);
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
				const auto query_toid = (notMapped) ? query_nodeid : mappingAux[target_toid];
				if (queryGraph.existEdge(query_nodeid, query_toid, tempEdge.label()) == false) return false;
			}
		}

		for (const auto& tempEdge : query_node.outEdges()) {
			const auto& query_toid = tempEdge.target();
			//this tempnode have been mapped
			const bool notMapped = IN_NODE_SET(queryStates[searchDepth].unmap, query_toid);
			if (notMapped && query_toid != query_nodeid) {
				const bool o = IN_NODE_SET(queryStates[searchDepth].out, query_toid);
				const bool i = IN_NODE_SET(queryStates[searchDepth].in, query_toid);
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
				const auto target_toid = (notMapped) ? target_nodeid : mapping[query_toid];
				if (targetGraph.existEdge(target_nodeid, target_toid, tempEdge.label()) == false)return false;
			}

		}

		clearNewCount();

		for (const auto& tempEdge : target_node.inEdges()) {
			const auto& target_fromid = tempEdge.source();
			const bool notMapped = IN_NODE_SET(targetState.unmap, target_fromid);
			if (notMapped && target_nodeid != target_fromid) {
				const bool o = IN_NODE_SET(targetState.out, target_fromid);
				const bool i = IN_NODE_SET(targetState.in, target_fromid);
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
				const auto query_fromid = (notMapped) ? query_nodeid : mappingAux[target_fromid];
				if (queryGraph.existEdge(query_fromid, query_nodeid, tempEdge.label()) == false)return false;
			}
		}

		for (const auto& tempEdge : query_node.inEdges()) {
			const auto& query_fromid = tempEdge.source();
			const bool notMapped = IN_NODE_SET(queryStates[searchDepth].unmap, query_fromid);
			if (notMapped && query_nodeid != query_fromid) {
				const bool o = IN_NODE_SET(queryStates[searchDepth].out, query_fromid);
				const bool i = IN_NODE_SET(queryStates[searchDepth].in, query_fromid);
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
				const auto target_fromid = (notMapped) ? target_nodeid : mapping[query_fromid];
				if (targetGraph.existEdge(target_fromid, target_nodeid, tempEdge.label()) == false)return false;
			}
			}
		return true;
		}


	State(const GraphType& _q, const GraphType& _t) : queryGraphPtr(&_q), targetGraphPtr(&_t), targetState(_t, _q.size())
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
public:
	State(const GraphType& _q, const GraphType& _t, shared_ptr<SubgraphMatchState<GraphType>[]> _queryStates) :State(_q, _t)
	{
		queryStates = _queryStates;
	};
	State(const GraphType& _q, const GraphType& _t, const vector<NodeIDType>& ms) :State(_q, _t) {
		queryStates = State<GraphType>::makeSubgraphState(_q, ms);
	};
	State() = default;

public:
	//public function
	vector<MapPair> calCandidatePairs(const NodeIDType id)const
	{
		vector<MapPair> answer;
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;
		const auto& queryNodeToMatchID = id;

		const bool queryNodeInIn = IN_NODE_SET(queryStates[searchDepth].in, queryNodeToMatchID);
		const bool queryNodeInOut = IN_NODE_SET(queryStates[searchDepth].out, queryNodeToMatchID);
		const auto& queryNode = queryGraph.node(queryNodeToMatchID);
		const auto queryNodeInRefTimes = queryStates[searchDepth].inRefTimes[queryNodeToMatchID];
		const auto queryNodeOutRefTimes = queryStates[searchDepth].outRefTimes[queryNodeToMatchID];
		const auto queryNodeInDepth = queryStates[searchDepth].inDepth[queryNodeToMatchID];
		const auto queryNodeOutDepth = queryStates[searchDepth].outDepth[queryNodeToMatchID];

		const auto queryNodeLabel = queryNode.label();

		const NodeIDType* begin = nullptr, * end = nullptr;
#ifdef NORMAL_ISO
		targetState.unmap.getSet(0, begin, end);
		for (auto it = begin; it < end; ++it) {
			const auto& targetNodeToMatchID = *it;
			const auto& targetNode = targetGraph.node(targetNodeToMatchID);
			if (queryNode.isSameType(targetNode) == false || queryNode > targetNode) continue;
			assert(mappingAux[targetNodeToMatchID] == NO_MAP);

			if (queryNodeInRefTimes > targetState.inRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeOutRefTimes > targetState.outRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeInDepth < targetState.inDepth[targetNodeToMatchID])continue;
			if (queryNodeOutDepth < targetState.outDepth[targetNodeToMatchID])continue;

			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));
		}

#endif
#if defined(INDUCE_ISO)
		if (queryNodeInIn) targetState.in.getSet(queryNodeInDepth, begin, end);
		else if (queryNodeInOut) targetState.out.getSet(queryNodeOutDepth, begin, end);
		else targetState.unmap.getSet(0, begin, end);
		answer.reserve(end - begin);
		for (auto it = begin; it < end; ++it)
		{
			const auto& targetNodeToMatchID = *it;
			const auto& targetNode = targetGraph.node(targetNodeToMatchID);
			if (queryNode.isSameType(targetNode) == false || queryNode > targetNode) continue;
			assert(mappingAux[targetNodeToMatchID] == NO_MAP);

			// it will be ditched because of sourceRule in next depth .
			if (queryNodeInRefTimes != targetState.inRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeOutRefTimes != targetState.outRefTimes[targetNodeToMatchID]) continue;
			if (queryNodeInDepth != targetState.inDepth[targetNodeToMatchID])continue;
			if (queryNodeOutDepth != targetState.outDepth[targetNodeToMatchID])continue;
			answer.push_back(MapPair(queryNodeToMatchID, targetNodeToMatchID));

		}
#endif
		return std::move(answer);

	}
	bool checkPair(const MapPair& cp)
	{
#ifdef INDUCE_ISO
		const bool answer = induceCheck(cp);
		//		const bool answer = sourceRule(cp) && targetRule(cp);
#elif defined(NORMAL_ISO)
		const bool answer = normalCheck(cp);
#endif
		return answer;
		}

	void pushPair(const MapPair& cp)
	{
		const auto targetNodeID = cp.second;
		const auto queryNodeID = cp.first;
		mapping[queryNodeID] = targetNodeID;
		mappingAux[targetNodeID] = queryNodeID;
		targetState.addNode(targetNodeID);
		searchDepth++;

		return;
	}
	void popPair(const MapPair& cp)
	{
		popPair(cp.first);
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

	static shared_ptr<SubgraphMatchState<GraphType>[]> makeSubgraphState(const GraphType& g, const vector<NodeIDType>& ms) {
		auto t1 = clock();
		assert(g.size() == ms.size());
		//the final state will never be used , so this function will not generate the final state;
		auto ptr = new SubgraphMatchState<GraphType>[ms.size()];
		if (ptr == nullptr) {
			cout << "allocate memory fail" << endl;
			exit(1);
		}

		shared_ptr<SubgraphMatchState<GraphType>[]> p(ptr);
		p[0] = move(SubgraphMatchState<GraphType>(g));
		LOOP(i, 1, ms.size()) {
			ptr[i] = ptr[i - 1];
			ptr[i].addNode(ms[i - 1]);
		}
		//		PRINT_TIME_COST_MS("time cost : ", clock() - t1);
		return p;
	}
	};


	}