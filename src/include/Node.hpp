#pragma once
#include"Edge.hpp"
#include<typeinfo>
#include<vector>

using namespace std;
template<typename _EdgeType, typename _NodeLabelType>
class Node {
public:
	typedef size_t NodeIDType;
	typedef _NodeLabelType NodeLabelType;
	typedef _EdgeType EdgeType;
	typedef Node<EdgeType, NodeLabelType> NodeType;
protected:
	NodeIDType id;

public:
	Node() = default;
	~Node() = default;
	Node(const NodeIDType _id) :id(_id) {}
	
	virtual const NodeLabelType& getLabel()const = 0;
	virtual void setLabel(const NodeLabelType _label) = 0;

/*	virtual bool operator==(const NodeType &n)const {
		return id == n.getID();
	}*/
	//same label
	virtual bool isSameType(const NodeType &n)const = 0;
	// >= operator :this node's edges should cover the second node's edges;
	// <= operator :similar to >= operator
	virtual bool operator>=(const NodeType &n) const = 0;
	virtual bool operator<=(const NodeType &n)const = 0;

	virtual bool existSameTypeEdgeToNode(const NodeType &n, const EdgeType& e)const = 0;
	virtual bool existSameTypeEdgeFromNode(const NodeType &n, const EdgeType& e)const = 0;
	virtual const vector<EdgeType>& getOutEdges() const = 0;
	virtual const vector<EdgeType>& getInEdges() const = 0;
	virtual size_t getOutEdgesNum() const = 0;
	virtual size_t getInEdgesNum() const = 0;
	virtual const NodeIDType& getID()const {
		return id;
	}
	// 
	virtual size_t nodeIdHash()const = 0;
	virtual void addInEdge(const EdgeType &e) = 0;
	virtual void addOutEdge(const EdgeType &e) = 0;
};

template<typename _EdgeType, typename _NodeLabelType>
class NodeVF2 :public Node<_EdgeType, _NodeLabelType> {
public:
	typedef size_t NodeIDType;
	typedef _NodeLabelType NodeLabelType;
	typedef _EdgeType EdgeType;
	typedef Node<EdgeType, NodeLabelType> NodeBaseType;
	typedef NodeVF2<EdgeType, NodeLabelType> NodeType;
	

private:

	NodeLabelType label = NodeLabelType();
	vector<EdgeType> inEdges, outEdges;

public:
	NodeVF2() = default;
	~NodeVF2() = default;
	NodeVF2(const NodeIDType _id ) :NodeBaseType(_id) {}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label) :NodeBaseType(_id), label(_label) {}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label, vector<EdgeType> &_inEdges, vector<EdgeType> &_outEdges) :NodeBaseType(_id,_label) {
		swap(inEdges, _inEdges);
		swap(outEdges, _outEdges);
	}
	  const NodeLabelType& getLabel()const {
	
		return label;
	}
	  void setLabel(const NodeLabelType _label) {
		this->label = _label;
	}
/*	  bool operator==(const NodeBaseType &n)const {
		  return this->getID() == n.getID();
	  }*/
	  bool isSameType(const NodeBaseType &n)const {	
		return label == n.getLabel();
	}
	  bool operator>=(const NodeBaseType &n) const {
		return ((inEdges.size() >= n.getInEdgesNum()) && (outEdges.size() >= n.getOutEdgesNum()));;
	}
	  bool operator<=(const NodeBaseType &n)const {
		return n >= *this;
	}
	  bool existSameTypeEdgeToNode(const NodeBaseType &n, const EdgeType& e)const {
		for (const auto &it : outEdges) {
			if (it.getTargetNodeID() == n.getID() && it.isSameTypeEdge(e)) return true;
		}
		return false;
	}
	  bool existSameTypeEdgeFromNode(const NodeBaseType &n, const EdgeType& e)const {
		for (const auto &it : inEdges) {
			if (it.getSourceNodeID() == n.getID() && it.isSameTypeEdge(e)) return true;
		}
		return false;
	}
	  const vector<EdgeType>& getOutEdges() const { return outEdges; }
	  const vector<EdgeType>& getInEdges() const { return inEdges; }
	  size_t getOutEdgesNum() const { return outEdges.size(); }
	  size_t getInEdgesNum() const { return inEdges.size(); }
	  size_t nodeIdHash()const { return hash<NodeIDType>()(this->getID()); }
	  void addInEdge(const EdgeType &e) { inEdges.push_back(e); }
	  void addOutEdge(const EdgeType &e) { outEdges.push_back(e); }
};

template<typename NodeType, typename NodeIDType = typename NodeType::NodeIDType>
static NodeIDType getNodeID(const NodeType &node) {
	return node.getID();
}
template<typename NodeType, typename NodeIDType = typename NodeType::NodeIDType>
static NodeIDType getNodeID(const NodeType *node) {
	return node->getID();
}
template<typename NodeType1, typename NodeType2>
static bool isSameTypeNode(const NodeType1 &n1, const NodeType2 &n2) {
	return n1.isSameType(n2);
}
namespace std
{
	template<typename EdgeType, typename NodeLabelType>
	struct hash<Node<EdgeType, NodeLabelType>>
	{
		size_t operator() (const Node<EdgeType, NodeLabelType>  &n) const {
			return n.nodeIdHash();
		}
	};
}
