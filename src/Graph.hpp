#pragma once
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
using namespace std;

template<typename NodeIDType>
class Graph {
	typedef Edge<NodeIDType> EdgeType;
	typedef Node<NodeIDType, EdgeType> NodeType;
	typedef const Node<NodeIDType, EdgeType>* NodeCPointer;
public:
	Graph() = default;
	~Graph() = default;
	virtual size_t graphSize() const {
		return 0;
	};
	virtual vector<NodeType> const & getAllNodes()const { return vector<NodeType>(); }
	virtual NodeCPointer getNodePointer(const NodeIDType &nodeID) {
		return new NodeCPointer();
	}
};
