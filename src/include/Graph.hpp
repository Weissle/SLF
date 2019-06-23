#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include"common.h"
#include<assert.h>
#include<unordered_map>
using namespace std;

template<typename _NodeType, typename _EdgeType>
class Graph {
public:
	typedef size_t	NodeIDType;
	typedef _NodeType NodeType;
	typedef _EdgeType EdgeType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef const NodeType* NodeCPointer;
public:
	Graph() = default;
	~Graph() = default;

	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel=EdgeLabelType()) = 0;
	virtual size_t size() const = 0;
	virtual void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) = 0;
	virtual vector<NodeType> const & nodes()const = 0;
	virtual const NodeType* getNodePointer(const NodeIDType &nodeID) const = 0;
	virtual const NodeType & getNode(const NodeIDType &nodeID) const = 0;
};

template<typename _NodeType, typename _EdgeType>
class GraphVF2 :public Graph<_NodeType, _EdgeType>
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef size_t NodeIDType;
	typedef _NodeType NodeType;
	typedef _EdgeType EdgeType;
	typedef GraphVF2<_NodeType, _EdgeType> GraphType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;
	
	//	typedef const NodeType* NodeCPointer;
private:

	vector<NodeType> _nodes;
//	unordered_map<NodeIDType, size_t> index;
	GRAPH_TYPE graphType;
	size_t _size;
public:
	GraphVF2() = default;
	GraphVF2(const vector<NodeType> &__nodes):_nodes(__nodes){
		_size=_nodes.size();
	}
	~GraphVF2() = default;

	GraphVF2(const size_t s, GRAPH_TYPE _graphType = GRAPH_TYPE::DIRECTION) :_size(s) , graphType(_graphType) {
		vector<NodeType> n;
		n.reserve(s + 1);
		for (auto i = 0; i < s; ++i) n.push_back(NodeType(i));
		swap(_nodes, n);

	}
	void edgeVectorReserve(const NodeIDType id,size_t s) {
		assert(id < _size);
		_nodes[id].reserve(s);
		return;
	}
	void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		assert(( _id < _size) && "node ID overflow");
		_nodes[_id].setLabel(_label);
		return;
	}
	void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel=EdgeLabelType()) {
		
	//	assert(source != target && "not support self loop");
		assert( source<_size && target<_size && "node id more than node num should be");
		auto &sourceNode = _nodes[source];
		auto &targetNode = _nodes[target];

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
	vector<NodeType> const & nodes()const { return _nodes; }
	const NodeType* getNodePointer(const NodeIDType &nodeID) const {
		assert((nodeID < _size) && "node ID overflow");
		return &_nodes[nodeID];
	}
	const NodeType & getNode(const NodeIDType &nodeID) const {
		assert((nodeID < _size )&& "node ID overflow");
		return _nodes[nodeID];
	}

	//do something to improve match speed
	void graphBuildFinish() {
		for (auto &node : _nodes) {
			node.NodeBuildFinish();
		}
	}

};
