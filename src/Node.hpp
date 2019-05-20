#pragma once
#include"Edge.hpp"
#include<xhash>
#include<vector>

using namespace std;
template<typename NodeIDType, typename EdgeType>
class Node {
	typedef Node<NodeIDType, EdgeType> NodeType;
public:

	//should id equal
	virtual bool operator==(const NodeType &n)const {
		return true;
	}
	//same label
	virtual bool isSameType(const NodeType &n)const {
		return true;
	}
	static bool isSameTypeNode(const NodeType &n1, const NodeType &n2) {
		return n1.isSameType(n2);
	}
	// something like edge
	virtual bool operator>=(const NodeType &n) const{
		return true;
	}
	virtual bool operator<=(const NodeType &n)const {
		return true;
	}
	virtual bool existSameTypeEdgeToNode (const NodeType &n, const EdgeType& e)const { return true; }
	virtual bool existSameTypeEdgeFromNode (const NodeType &n, const EdgeType& e)const { return true; }

	virtual const vector<EdgeType>& getOutEdges() const{ return vector<EdgeType>(); }
	virtual const vector<EdgeType>& getInEdges() const{ return vector<EdgeType>(); }
	virtual size_t nodeIdHash()const { return 0; }
};

namespace std
{
	template<typename NodeIDType, typename EdgeType>
	struct hash<Node<NodeIDType, EdgeType>>
	{
		size_t operator() (const Node  &n) const {
			return n.nodeIdHash();
		}
	};
}
