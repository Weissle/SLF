#pragma once
#include"Edge.hpp"
#include"common.h"
#include<algorithm>
#include<typeinfo>
#include<vector>
#include<iostream>
#include<time.h>
using namespace std;
template<typename _EdgeType>
class Node {
public:

	typedef size_t NodeIDType;
	typedef size_t NodeLabelType;
	typedef _EdgeType EdgeType;
	typedef Node<EdgeType> NodeType;
protected:
	NodeIDType _id = UINT32_MAX;

public:
	Node() = default;
	~Node() = default;
	Node(const NodeIDType __id) :_id(__id) {}

	virtual const NodeLabelType& getLabel()const = 0;
	virtual void setLabel(const NodeLabelType _label) = 0;
	/*	virtual bool operator==(const NodeType &n)const {
			return id == n.id;
		}*/
		//same label
	virtual const NodeIDType id()const { return _id; }
	virtual bool isSameType(const NodeType& n)const = 0;
	// >= operator :this node's edges should cover the second node's edges;
	// <= operator :similar to >= operator
	virtual bool operator>=(const NodeType& n) const = 0;
	virtual bool operator<=(const NodeType& n)const = 0;
	virtual bool existSameTypeEdgeToNode(const NodeType& n, const EdgeType& e)const = 0;
	virtual bool existSameTypeEdgeFromNode(const NodeType& n, const EdgeType& e)const = 0;
	virtual const vector<EdgeType>& getOutEdges() const = 0;
	virtual const vector<EdgeType>& getInEdges() const = 0;
	virtual size_t getOutEdgesNum() const = 0;
	virtual size_t getInEdgesNum() const = 0;
	virtual void reserve(const size_t s) = 0;
	virtual void NodeBuildFinish() = 0;
	virtual size_t nodeIdHash()const = 0;
	virtual void addInEdge(const EdgeType& e) = 0;
	virtual void addOutEdge(const EdgeType& e) = 0;
};

template<typename _EdgeType>
class NodeVF2 :public Node<_EdgeType> {
public:
	typedef size_t NodeIDType;
	typedef size_t NodeLabelType;
	typedef _EdgeType EdgeType;
	typedef Node<EdgeType> NodeBaseType;
	typedef NodeVF2<EdgeType> NodeType;


private:

	NodeLabelType label = NodeLabelType();
	vector<EdgeType> inEdges, outEdges;
	bool edgeSort = false;
public:
	NodeVF2() = default;
	~NodeVF2() = default;
	NodeVF2(const NodeIDType _id) :NodeBaseType(_id) {}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label) :NodeBaseType(_id), label(_label) {}
	NodeVF2(const NodeIDType _id, const NodeLabelType _label, vector<EdgeType>& _inEdges, vector<EdgeType>& _outEdges) :NodeType(_id, _label) {
		swap(inEdges, _inEdges);
		swap(outEdges, _outEdges);
	}

	const NodeLabelType& getLabel()const {

		return label;
	}
	void setLabel(const NodeLabelType _label) {
		this->label = _label;
	}
	bool isSameType(const NodeBaseType& n)const {
		return label == n.getLabel();
	}
	bool operator>=(const NodeBaseType& n) const {
		return ((inEdges.size() >= n.getInEdgesNum()) && (outEdges.size() >= n.getOutEdgesNum()));;
	}
	bool operator<=(const NodeBaseType& n)const {
		return n >= *this;
	}
	bool existSameTypeEdgeToNode(const NodeBaseType& n, const EdgeType& e)const {
		if (edgeSort) {
			return binary_search(outEdges.begin(), outEdges.end(), EdgeType(EdgeType::NODE_RECORD_TYPE::TARGET, this->id(), n.id(), e.getLabel()));
		}
		else {

			for (const auto& it : outEdges) {
				if (it.getTargetNodeID() == n.id() && it.isSameTypeEdge(e))return true;
			}
			return false;
		}
	}
	bool existSameTypeEdgeFromNode(const NodeBaseType& n, const EdgeType& e)const {
		if (edgeSort) {
			return binary_search(inEdges.begin(), inEdges.end(), EdgeType(EdgeType::NODE_RECORD_TYPE::SOURCE, n.id(), this->id(), e.getLabel()));
		}
		else {
			for (const auto& it : inEdges) {
				if (it.getSourceNodeID() == n.id() && it.isSameTypeEdge(e)) return true;
			}
			return false;
		}
	}
	void reserve(const size_t s) {
		inEdges.reserve(s);
		outEdges.reserve(s);
	}
	const vector<EdgeType>& getOutEdges() const { return outEdges; }
	const vector<EdgeType>& getInEdges() const { return inEdges; }
	size_t getOutEdgesNum() const { return outEdges.size(); }
	size_t getInEdgesNum() const { return inEdges.size(); }
	size_t nodeIdHash()const { return hash<NodeIDType>()(this->_id); }
	void addInEdge(const EdgeType& e) {
		inEdges.push_back(e);
		edgeSort = false;
	}
	void addOutEdge(const EdgeType& e) {
		outEdges.push_back(e);
		edgeSort = false;
	}
	void NodeBuildFinish() {
		inEdges.shrink_to_fit();
		outEdges.shrink_to_fit();
		sort(inEdges.begin(), inEdges.end(), [](const EdgeType& a, const EdgeType& b) {
			return a < b;
			});
		sort(outEdges.begin(), outEdges.end(), [](const EdgeType& a, const EdgeType& b) {
			return a < b;
			});
		edgeSort = true;
	}
};


template<typename NodeType1, typename NodeType2>
static bool isSameTypeNode(const NodeType1& n1, const NodeType2& n2) {
	return n1.isSameType(n2);
}
namespace std
{
	template<typename EdgeType>
	struct hash<Node<EdgeType>>
	{
		size_t operator() (const Node<EdgeType>& n) const {
			return n.nodeIdHash();
		}
	};
}
