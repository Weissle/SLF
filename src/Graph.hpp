#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include<unordered_map>
using namespace std;

template<typename NodeIDType, typename EdgeLabelType, typename NodeLabelType>
class Graph {
	typedef Edge<NodeIDType, EdgeLabelType> EdgeType;
	typedef Node<NodeIDType, EdgeType, NodeLabelType> NodeType;
	typedef const Node<NodeIDType, EdgeType, NodeLabelType>* NodeCPointer;
public:
	Graph() = default;
	~Graph() = default;
	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) { return; }
	virtual void addEdge(const NodeIDType source, const NodeIDType target) { return; }
	virtual size_t graphSize() const {
		return 0;
	};
	virtual vector<NodeType> const & getAllNodes()const { return vector<NodeType>(); }
	virtual NodeCPointer getNodePointer(const NodeIDType &nodeID) {
		return new NodeCPointer();
	}
};

template<typename NodeIDType, typename EdgeLabelType, typename NodeLabelType>
class GraphVF2 :Graph<NodeIDType, EdgeLabelType, NodeLabelType>
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
private:
	typedef Edge<NodeIDType, EdgeLabelType> EdgeType;
	typedef Node<NodeIDType, EdgeType, NodeLabelType> NodeType;
	typedef const Node<NodeIDType, EdgeType, NodeLabelType>* NodeCPointer;
	vector<NodeType> nodes;
	unordered_map<NodeIDType, NodeCPointer> index;
	GRAPH_TYPE graphType;
	size_t graphsize;
	size_t calUOS_reserveSize(size_t need) {
		size_t i = 16;
		while (i < need) i = i << 1;
		if (i * 0.9 > need) return i;
		else return i << 1;
	}
public:
	GraphVF2() = default;
	~GraphVF2() = default;
	GraphVF2(vector<NodeType> &_nodes, GRAPH_TYPE _graphType = GRAPH_TYPE::DIRECTION) 
		:nodes(_nodes), graphType(__graphType) 
	{
		graphsize = nodes.size();
		index.reserve(calUOS_reserveSize(nodes.size()));
		for (const auto &it : nodes) {
			const NodeIDType &id = it.getID();
			index[id] = &it;
		}
	}
	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) {
		const auto &sourceNodePointer = index[source];
		const auto &targetNodePointer = index[target];
		const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, source, target, edgeLabel);
		sourceNodePointer->addOutEdge(target->getID(), tempEdge);
		targetNodePointer->addInEdge(source->getID(), tempEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, target, source, edgeLabel);
			sourceNodePointer->addInEdge(target->getID(), tempEdge);
			targetNodePointer->addOutEdge(source->getID(), tempEdge);
		}
	}
	virtual void addEdge(const NodeIDType source, const NodeIDType target) {
		const auto &sourceNodePointer = index[source];
		const auto &targetNodePointer = index[target];
		const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, source, target);
		sourceNodePointer->addOutEdge(target->getID(), tempEdge);
		targetNodePointer->addInEdge(source->getID(), tempEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, target, source);
			sourceNodePointer->addInEdge(target->getID(), tempEdge);
			targetNodePointer->addOutEdge(source->getID(), tempEdge);
		}
	}
	virtual size_t graphSize() const {
		return graphsize;
	};
	virtual vector<NodeType> const & getAllNodes()const { return nodes; }
	virtual NodeCPointer getNodePointer(const NodeIDType &nodeID) {
		return index[nodeID];
	}

};
