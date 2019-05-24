#pragma once
#include"Edge.hpp"
#include<xhash>
#include<typeinfo>
#include<vector>

using namespace std;
template<typename NodeIDType, typename EdgeType, typename NodeLabelType>
class Node {
	NodeIDType id;

public:
	typedef typename NodeIDType NodeIDType;
	typedef typename NodeLabelType NodeLabelType;
	typedef Node<NodeIDType, EdgeType, NodeLabelType> NodeType;
public:
	Node() = default;
	~Node() = default;
	Node(const NodeIDType _id) :id(_id) {}
	//should id equal
	virtual bool operator==(const NodeType &n)const {
		return id == n.getID();
	}
	virtual void setLabel(const NodeLabelType _label) {
		return;
	}
	//same label
	virtual const NodeIDType& getID()const {
		return id;
	}
	virtual const NodeLabelType& getLabel()const {
		throw "Node.hpp this Node class should not have label";
		return NodeLabelType();
	}
	virtual bool isSameType(const NodeType &n)const {
		return true;
	}
	static bool isSameTypeNode(const NodeType &n1, const NodeType &n2) {
		return n1.isSameType(n2);
	}
	// something like edge
	virtual bool operator>=(const NodeType &n) const {
		return true;
	}
	virtual bool operator<=(const NodeType &n)const {
		return true;
	}
	virtual bool existSameTypeEdgeToNode(const NodeType &n, const EdgeType& e)const { return true; }
	virtual bool existSameTypeEdgeFromNode(const NodeType &n, const EdgeType& e)const { return true; }

	virtual const vector<EdgeType>& getOutEdges() const { return vector<EdgeType>(); }
	virtual const vector<EdgeType>& getInEdges() const { return vector<EdgeType>(); }
	virtual size_t getOutEdgesNum() const { return 0; }
	virtual size_t getInEdgesNum() const { return 0; }

	virtual size_t nodeIdHash()const { return 0; }

	virtual void addInEdge(const EdgeType e) { return; }
	virtual void addOutEdge(const EdgeType e) { return; }
};

namespace std
{
	template<typename NodeIDType, typename EdgeType, typename NodeLabelType>
	struct hash<Node<NodeIDType, EdgeType, NodeLabelType>>
	{
		size_t operator() (const Node  &n) const {
			return n.nodeIdHash();
		}
	};
}

template<typename NodeIDType, typename EdgeType, typename NodeLabelType>
class NodeVF2 :public Node<NodeIDType, EdgeType, NodeLabelType> {
public:
	typedef NodeVF2<NodeIDType, EdgeType, NodeLabelType> NodeType;
	typedef typename NodeIDType NodeIDType;
	typedef typename NodeLabelType NodeLabelType;
	typedef NodeVF2<NodeIDType, EdgeType, NodeLabelType> NodeType;
private:

	NodeLabelType label = NodeLabelType();
	vector<EdgeType> inEdges, outEdges;

public:
	NodeVF2() = default;
	~NodeVF2() = default;
	NodeVF2(const NodeIDType _id ) :Node<NodeIDType,EdgeType,NodeLabelType>(_id) {}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label) :NodeType(_id), label(_label) {}
	NodeVF2(const NodeIDType _id,const NodeLabelType _label,const size_t edgeNum) :NodeType(_id), label(_label) {
		inEdges.reserve(edgeNum);
		outEdges.reserve(edgeNum);
	}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label, vector<EdgeType> &_inEdges, vector<EdgeType> &_outEdges) :NodeType(_id), label(_label) {
		swap(inEdges, _inEdges);
		swap(outEdges, _outEdges);
	}
	virtual const NodeLabelType& getLabel()const {
	
		return label;
	}
	virtual void setLabel(const NodeLabelType _label) {
		this->label = _label;
	}
	virtual bool isSameType(const NodeType &n)const {
	
		return label == n.getLabel();
	}
	virtual bool operator>=(const NodeType &n) const {
		return ((inEdges.size() >= n.getInEdgesNum()) && (outEdges.size() >= n.getOutEdgesNum()));;
	}
	virtual bool operator<=(const NodeType &n)const {
		return n >= *this;
	}
	virtual bool existSameTypeEdgeToNode(const NodeType &n, const EdgeType& e)const {
		for (const auto &it : outEdges) {
			if (it.getTargetNodeID() == n.getID() && it.isSameTypeEdge(e)) return true;
		}
		return false;
	}
	virtual bool existSameTypeEdgeFromNode(const NodeType &n, const EdgeType& e)const {
		for (const auto &it : inEdges) {
			if (it.getSourceNodeID() == n.getID() && it.isSameTypeEdge(e)) return true;
		}
		return false;
	}
	virtual const vector<EdgeType>& getOutEdges() const { return outEdges; }
	virtual const vector<EdgeType>& getInEdges() const { return inEdges; }
	virtual size_t getOutEdgesNum() const { return outEdges.size(); }
	virtual size_t getInEdgesNum() const { return inEdges.size(); }
	virtual size_t nodeIdHash()const { return hash<NodeIDType>()(this->getID()); }
	virtual void addInEdge(const EdgeType &e) { inEdges.push_back(e); }
	virtual void addOutEdge(const EdgeType &e) { outEdges.push_back(e); }
};