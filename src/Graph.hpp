#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include<unordered_map>
using namespace std;

template<typename NodeType, typename EdgeType>
class Graph {
public:
	typedef typename NodeType NodeType;
	typedef typename EdgeType EdgeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

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
	virtual NodeType* getNodePointer(const NodeIDType &nodeID) const{
		return NodeCPointer();
	}
};

template<typename NodeType, typename EdgeType>
class GraphVF2 :Graph<NodeType, EdgeType>
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef typename NodeType NodeType;
	typedef typename EdgeType EdgeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	typedef const Node<NodeIDType, EdgeType, NodeLabelType>* NodeCPointer;
private:

	vector<NodeType> nodes;
	unordered_map<NodeIDType, NodeType*> index;
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
		auto &sourceNodePointer = index[source];
		auto &targetNodePointer = index[target];
		const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, source, target);
		sourceNodePointer->addOutEdge(tempEdge);
		targetNodePointer->addInEdge(tempEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType tempEdge = EdgeVF2<NodeIDType, EdgeLabelType>(EdgeVF2::NODE_RECORD_TYPE::BOTH, target, source);
			sourceNodePointer->addInEdge(tempEdge);
			targetNodePointer->addOutEdge(tempEdge);
		}
	}
	virtual size_t graphSize() const {
		return graphsize;
	};
	virtual vector<NodeType> const & getAllNodes()const { return nodes; }
	virtual NodeType* getNodePointer (const NodeIDType &nodeID) const {
	//	return NodeCPointer();
		return index[nodeID];
	//	index.find(nodeID)->second;
	}

};
