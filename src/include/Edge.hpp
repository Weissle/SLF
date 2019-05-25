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
	virtual const NodeIDType& getSourceNodeID()const = 0;
	virtual const NodeIDType& getTargetNodeID()const = 0;
	virtual bool operator==(const EdgeType &e)const = 0;
	virtual bool isSameTypeEdge(const EdgeType &n) const = 0;
	virtual const EdgeLabelType&  getLabel()const = 0;
};


template<typename NodeIDType, typename EdgeLabelType>
class EdgeVF2 :public Edge<NodeIDType, EdgeLabelType> {

public:
	enum NODE_RECORD_TYPE { SOURCE, TARGET, BOTH };
	typedef typename EdgeLabelType EdgeLabelType;
	typedef typename EdgeVF2<NodeIDType, EdgeLabelType> EdgeType;
	typedef typename Edge<NodeIDType, EdgeLabelType> EdgeBaseType;
private:
	NodeIDType source, target;
	NODE_RECORD_TYPE recodeType;
	EdgeLabelType label = EdgeLabelType();
public:
	EdgeVF2() = default;
	~EdgeVF2() = default;
	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _source, const NodeIDType _target) :
		recodeType(_recodeType), source(_source), target(_target) {
	}
	EdgeVF2(NODE_RECORD_TYPE _recodeType, const NodeIDType _source, const NodeIDType _target, const EdgeLabelType _label) :
		recodeType(_recodeType), source(_source), target(_target), label(_label) {
	}
	const NodeIDType& getSourceNodeID() const {
		if (recodeType == NODE_RECORD_TYPE::TARGET) throw "this is a edge record target node!!";
		return source;
	}
	const NodeIDType& getTargetNodeID()const
	{
		if (recodeType == NODE_RECORD_TYPE::SOURCE)throw "this is a edge record source node!!";
		return target;
	}
	const EdgeLabelType&  getLabel()const
	{
		return label;
	}
	bool isSameTypeEdge(const EdgeBaseType &n) const {
		return label == n.getLabel();
	}
	bool operator==(const EdgeBaseType &e)const {
		return this->isSameTypeEdge(e);
	}
};

