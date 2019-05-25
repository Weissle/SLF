#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include<assert.h>
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

	typedef const NodeType* NodeCPointer;
public:
	Graph() = default;
	~Graph() = default;
	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) { return; }
	virtual void addEdge(const NodeIDType source, const NodeIDType target) { return; }
	virtual size_t graphSize() const {
		return 0;
	};
	virtual void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		return;
	}
	virtual vector<NodeType> const & getAllNodes()const { return vector<NodeType>(); }
	virtual const NodeType* getNodePointer(const NodeIDType &nodeID) const {
		return nullptr;
	}
	virtual const NodeType & getNode(const NodeIDType &nodeID) const {
		return NodeType();
	}

};

template<typename NodeType, typename EdgeType>
class GraphVF2 :public Graph<NodeType, EdgeType>
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	typedef typename NodeType NodeType;
	typedef typename EdgeType EdgeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;
	//	typedef const NodeType* NodeCPointer;
private:

	vector<NodeType> nodes;
	//	unordered_map<NodeIDType, NodeType*> index;
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
			if (i * 0.9 > need) return i;
			else return i << 1;
		};
		graphsize = nodes.size();
		index.reserve(calASizeForHash(nodes.size()));
		for (auto i = 0; i < nodes.size(); ++i) {
			index[nodes[i].getID()] = i;
		}
		/*	for (auto &it : nodes) {
				auto temp = &it;
				NodeIDType id = it.getID();
				index[id] = &it;
			}*/
	}
	virtual void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		auto &tempNode = nodes[index[_id]];
		//	tempNode->setLabel(_label);
		tempNode.setLabel(_label);
		return;
	}
	virtual void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel) {
		auto &sourceNode = nodes[index[source]];
		auto &targetNode = nodes[index[target]];
		const EdgeType tempEdge = EdgeType(EdgeType::NODE_RECORD_TYPE::BOTH, source, target, edgeLabel);
		sourceNode.addOutEdge(tempEdge);
		targetNode.addInEdge(tempEdge);
		if (GRAPH_TYPE::BIDIRECTION == graphType) {
			const EdgeType tempEdge = EdgeType(EdgeType::NODE_RECORD_TYPE::BOTH, target, source, edgeLabel);
			sourceNode.addInEdge(tempEdge);
			targetNode.addOutEdge(tempEdge);
		}
	}
	virtual void addEdge(const NodeIDType source, const NodeIDType target) {
		addEdge(source, target, EdgeLabelType());
	}
	virtual size_t graphSize() const {
		return graphsize;
	};
	virtual vector<NodeType> const & getAllNodes()const { return nodes; }
	virtual const NodeType* getNodePointer(const NodeIDType &nodeID) const {
		const auto tempIndexPair = index.find(nodeID);
		assert((tempIndexPair != index.end()) && "this node id not exist!");
		const auto nodeIndex = tempIndexPair->second;
		return &(nodes[nodeIndex]);
	}
	virtual const NodeType & getNode(const NodeIDType &nodeID) const {
		const auto tempIndexPair = index.find(nodeID);
		assert((tempIndexPair != index.end()) && "this node id not exist!");
		const auto nodeIndex = tempIndexPair->second;
		return nodes[nodeIndex];
	}

};
