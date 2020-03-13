#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include"common.h"
#include<assert.h>
#include<unordered_map>
#include<utility>
using namespace std;
namespace wg {
template<typename NodeType, typename EdgeType>
class Graph
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef size_t NodeIDType;
	typedef Graph<NodeType, EdgeType> GraphType;
	typedef NodeType NodeType;
	typedef EdgeType EdgeType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

private:

	vector<NodeType> _nodes;
	GRAPH_TYPE graphType;
	size_t _size;

	// NodeLabel -> the maximum out and in degrees of this kind of nodes have;
	unordered_map<NodeLabelType, size_t> aux_LabelMaxOut;
	unordered_map<NodeLabelType, size_t> aux_LabelMaxIn;
	// NodeLabel -> quantity of nodes have this label
	unordered_map<NodeLabelType, size_t> aux_LabelNum;
	NodeLabelType _maxLabel = 0;
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

		const EdgeType  sourceEdge = EdgeType(EDGE_RECORD_TYPE::SOURCE, source, target, edgeLabel);
		const EdgeType  targetEdge = EdgeType(EDGE_RECORD_TYPE::TARGET, source, target, edgeLabel);
		sourceNode.addOutEdge(targetEdge);
		targetNode.addInEdge(sourceEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType sourceEdge1 = EdgeType(EDGE_RECORD_TYPE::SOURCE, target, source, edgeLabel);
			const EdgeType targetEdge1 = EdgeType(EDGE_RECORD_TYPE::TARGET, target, source, edgeLabel);
			sourceNode.addInEdge(sourceEdge1);
			targetNode.addOutEdge(targetEdge1);
		}
	}

	size_t size() const {
		return _size;
	};
	const vector<NodeType>& nodes()const { return _nodes; }

	const NodeType& node(const NodeIDType& nodeID) const {
		assert((nodeID < _size) && "node ID overflow");
		return _nodes[nodeID];
	}
	unordered_map<NodeLabelType,size_t> labelMaxIn()const {
		return this->aux_LabelMaxIn;
	}
	unordered_map<NodeLabelType, size_t> labelMaxOut()const {
		return this->aux_LabelMaxOut;
	}
	unordered_map<NodeLabelType, size_t> labelNum()const {
		return this->aux_LabelNum;
	}
	//do something to improve match speed
	void graphBuildFinish() {


		//only allow label type from 0 1 2 3 ... n-1
		for (auto& node : _nodes) {
			node.NodeBuildFinish();
			auto nodeLabel = node.label();
			this->aux_LabelNum[nodeLabel]++;

			auto &inMax = aux_LabelMaxIn[nodeLabel];
			inMax = max(inMax, node.inEdgesNum());
			auto& outMax = aux_LabelMaxOut[nodeLabel];
			outMax = max(outMax, node.outEdgesNum());
			_maxLabel = max(_maxLabel, nodeLabel);
		}

	}
	NodeLabelType maxLabel()const { return _maxLabel; }
	bool existEdge(const NodeIDType& from, const NodeIDType& to, const EdgeLabelType& edgeLabel)const {
		return _nodes[from].existSameTypeEdgeToNode(to, edgeLabel);
	}
	const NodeType& operator[](const NodeIDType& nodeID)const {
		return node(nodeID);
	}

};

}