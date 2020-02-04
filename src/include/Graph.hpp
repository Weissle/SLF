#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include"common.h"
#include<Pair.hpp>
#include<assert.h>
#include<map>
#include<unordered_map>
#include<utility>
using namespace std;
namespace wg {
template<typename _NodeType, typename _EdgeType>
class Graph
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef size_t NodeIDType;
	typedef _NodeType NodeType;
	typedef _EdgeType EdgeType;
	typedef Graph<_NodeType, _EdgeType> GraphType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

private:

	vector<NodeType> _nodes;
	GRAPH_TYPE graphType;
	size_t _size;

	// NodeLabel -> the maximum out and in degrees of this kind of nodes have;
	unordered_map<NodeLabelType, pair<size_t, size_t>> auxLDinform;
	// NodeLabel -> quantity of nodes have this label
	unordered_map<NodeLabelType, size_t> auxLQinform;
public:
	Graph() = default;
	Graph(const vector<NodeType>& __nodes) :_nodes(__nodes) {
		_size = _nodes.size();
	}
	~Graph() = default;

	Graph(const size_t s, GRAPH_TYPE _graphType = GRAPH_TYPE::DIRECTION) :_size(s), graphType(_graphType) {
		vector<NodeType> n;
		n.reserve(s + 1);
		for (auto i = 0; i < s; ++i) n.push_back(NodeType(i));
		swap(_nodes, n);

	}
	void edgeVectorReserve(const NodeIDType id, size_t s) {
		assert(id < _size);
		_nodes[id].reserve(s);
		return;
	}
	void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		assert((_id < _size) && "node ID overflow");
		_nodes[_id].setLabel(_label);
		return;
	}
	void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel = EdgeLabelType()) {

		//	assert(source != target && "not support self loop");
		assert(source < _size && target < _size && "node id should smaller than node number");
		auto& sourceNode = _nodes[source];
		auto& targetNode = _nodes[target];

		const EdgeType  sourceEdge = EdgeType(EdgeType::NODE_RECORD_TYPE::SOURCE, source, target, edgeLabel);
		const EdgeType  targetEdge = EdgeType(EdgeType::NODE_RECORD_TYPE::TARGET, source, target, edgeLabel);
		sourceNode.addOutEdge(targetEdge);
		targetNode.addInEdge(sourceEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType sourceEdge1 = EdgeType(EdgeType::NODE_RECORD_TYPE::SOURCE, target, source, edgeLabel);
			const EdgeType targetEdge1 = EdgeType(EdgeType::NODE_RECORD_TYPE::TARGET, target, source, edgeLabel);
			sourceNode.addInEdge(sourceEdge1);
			targetNode.addOutEdge(targetEdge1);
		}
	}

	size_t size() const {
		return _size;
	};
	const vector<NodeType>& nodes()const { return _nodes; }
	const NodeType* nodePointer(const NodeIDType& nodeID) const {
		assert((nodeID < _size) && "node ID overflow");
		return &_nodes[nodeID];
	}
	const NodeType& node(const NodeIDType& nodeID) const {
		assert((nodeID < _size) && "node ID overflow");
		return _nodes[nodeID];
	}
	unordered_map<NodeLabelType, pair<size_t, size_t>> LDinform()const {
		return auxLDinform;
	}
	unordered_map<NodeLabelType, size_t> LQinform()const {
		return auxLQinform;
	}
	//do something to improve match speed
	void graphBuildFinish() {
		size_t labelTypeNum = 0, labelMax = 0;

		//only allow label type from 1 2 3 ... n-1
		for (auto& node : _nodes) {
			node.NodeBuildFinish();
			//std::map<NodeLabelType,FSPair<int,int>>
			auto po = auxLDinform.find(node.label());
			auxLQinform[node.label()]++;
			if (po == auxLDinform.end()) {
				auxLDinform[node.label()].first = node.outEdgesNum();
				auxLDinform[node.label()].second = node.inEdgesNum();
				labelTypeNum++;
				labelMax = max(labelMax, node.label());
			}
			else {
				po->second.first = max(po->second.first, node.outEdgesNum());
				po->second.second = max(po->second.second, node.inEdgesNum());
			}
		}
		assert(labelTypeNum - 1 == labelMax && "n kinds of node label , nodes' label type should range from 0 to n-1 ");

	}
	bool existEdge(const NodeIDType& from, const NodeIDType& to, const EdgeLabelType& edgeLabel)const {
		return _nodes[from].existSameTypeEdgeToNode(to, edgeLabel);
	}
	const NodeType& operator[](const NodeIDType& nodeID)const {
		return node(nodeID);
	}

};

}