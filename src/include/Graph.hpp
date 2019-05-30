#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include<assert.h>
#include<unordered_map>
using namespace std;

template<typename _NodeType, typename _EdgeType>
class Graph {
public:
	typedef _NodeType NodeType;
	typedef _EdgeType EdgeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef const NodeType* NodeCPointer;
public:
	Graph() = default;
	~Graph() = default;

	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) = 0;
	virtual void addEdge(const NodeIDType source, const NodeIDType target) = 0;
	virtual size_t graphSize() const = 0;
	virtual void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) = 0;
	virtual vector<NodeType> const & getAllNodes()const = 0;
	virtual const NodeType* getNodePointer(const NodeIDType &nodeID) const = 0;
	virtual const NodeType & getNode(const NodeIDType &nodeID) const = 0;

};

template<typename _NodeType, typename _EdgeType>
class GraphVF2 :public Graph<_NodeType, _EdgeType>
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef _NodeType NodeType;
	typedef _EdgeType EdgeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;
	//	typedef const NodeType* NodeCPointer;
private:

	vector<NodeType> nodes;
	unordered_map<NodeIDType, size_t> index;
	GRAPH_TYPE graphType;
	size_t graphsize;
public:
	GraphVF2() = default;
	~GraphVF2() = default;
	GraphVF2(vector<NodeType> &_nodes, GRAPH_TYPE _graphType = GRAPH_TYPE::DIRECTION)
		:nodes(_nodes), graphType(_graphType)
	{
		auto calASizeForHash = [](const size_t need) {
			size_t i = 16;
			while (i < need) i = i << 1;
			if (i * 0.8 > need) return i;
			else return i << 1;
		};
		graphsize = nodes.size();
		index.reserve(calASizeForHash(nodes.size()));
		for (size_t i = 0; i < nodes.size(); ++i) {
			index[nodes[i].getID()] = i;
		}
	}
	void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		auto &tempNode = nodes[index[_id]];
		tempNode.setLabel(_label);
		return;
	}
	void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) {
		auto &sourceNode = nodes[index[source]];
		auto &targetNode = nodes[index[target]];

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
	void addEdge(const NodeIDType source, const NodeIDType target) {
		addEdge(source, target, EdgeLabelType());
	}
	size_t graphSize() const {
		return graphsize;
	};
	vector<NodeType> const & getAllNodes()const { return nodes; }
	const NodeType* getNodePointer(const NodeIDType &nodeID) const {
		const auto tempIndexPair = index.find(nodeID);
		assert((tempIndexPair != index.end()) && "this node id not exist!");
		const auto nodeIndex = tempIndexPair->second;
		return &(nodes[nodeIndex]);
	}
	const NodeType & getNode(const NodeIDType &nodeID) const {
		const auto tempIndexPair = index.find(nodeID);
		assert((tempIndexPair != index.end()) && "this node id not exist!");
		const auto nodeIndex = tempIndexPair->second;
		return nodes[nodeIndex];
	}

};
