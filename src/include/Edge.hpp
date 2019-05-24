#pragma once
#include<typeinfo>
template<typename NodeIDType, typename EdgeLabelType>
class Edge {
public:
	typedef typename EdgeLabelType EdgeLabelType;
	typedef typename Edge<NodeIDType, EdgeLabelType> EdgeType;
public:
	Edge() = default;
	~Edge() = default;
	virtual const NodeIDType& getSourceNodeID() const { return NodeIDType(); }
	virtual const NodeIDType& getTargetNodeID()const { return NodeIDType(); }
	virtual bool isSameTypeEdge(const EdgeType &n) const { return true; }
	virtual const EdgeLabelType&  getLabel()const {
		throw "this edge have no label";
		return EdgeLabelType();
	}
	/*	virtual bool operator==(const Edge<NodeIDType> &e) {
			return true;
		}*/
};

//edge and two node
template<typename NodeIDType, typename EdgeLabelType>
class EdgeVF2 :public Edge<NodeIDType, EdgeLabelType> {

public:
	enum NODE_RECORD_TYPE { SOURCE, TARGET, BOTH };
	typedef typename EdgeLabelType EdgeLabelType;
	typedef typename Edge<NodeIDType, EdgeLabelType> EdgeType;
private:
	NodeIDType source, target;
	NODE_RECORD_TYPE recodeType;
	EdgeLabelType label = EdgeLabelType();
public:


	EdgeVF2() = default;
	~EdgeVF2() = default;
	virtual const NodeIDType& getSourceNodeID() const {
		if (recodeType == NODE_RECORD_TYPE::TARGET) throw "this is a edge record target node!!";
		return source;
	}
	virtual const NodeIDType& getTargetNodeID()const
	{
		if (recodeType == NODE_RECORD_TYPE::SOURCE)throw "this is a edge record source node!!";
		return target;
	}
	virtual const EdgeLabelType&  getLabel()const
	{

		return label;
	}
	virtual bool isSameTypeEdge(const EdgeType &n) const {
		return label == n.getLabel();
	}
	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _node) :recodeType(_recodeType) {
		if (_recodeType == NODE_RECORD_TYPE::BOTH) throw "not enough paramete for edge";
		else if (_recodeType == NODE_RECORD_TYPE::SOURCE) source = _node;
		else if (_recodeType == NODE_RECORD_TYPE::TARGET) target = _node;
		else throw "throw in Edge.hpp EdgeVF2 class";
		if (typeid(EdgeLabelType) != typeid(void)) throw "Edge.hpp EdgeVF2 should have a label";
	}
	/*	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _node,const EdgeLabelType _label) :recodeType(_recodeType),label(_label) {
			if (_recodeType == NODE_RECORD_TYPE::BOTH) throw "not enough paramete for edge";
			else if (_recodeType == NODE_RECORD_TYPE::SOURCE) source = _node;
			else if (_recodeType == NODE_RECORD_TYPE::TARGET) target = _node;
			else throw "throw in Edge.hpp EdgeVF2 class";
		}*/
	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _source, const NodeIDType _target) :
		recodeType(_recodeType), source(_source), target(_target) {
	}
	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _source, const NodeIDType _target, const EdgeLabelType _label) :
		recodeType(_recodeType), source(_source), target(_target), label(_label) {
		if (typeid(EdgeLabelType) != typeid(void)) throw "Edge.hpp EdgeVF2 should have a label";
	}
};

