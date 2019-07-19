#pragma once
#include<typeinfo>
#include<assert.h>
template<typename _EdgeLabelType>
class Edge {
public:
	typedef _EdgeLabelType EdgeLabelType;
	typedef size_t NodeIDType;
	typedef Edge<EdgeLabelType> EdgeType;
public:
	Edge() = default;
	~Edge() = default;
	virtual const NodeIDType& getSourceNodeID()const = 0;
	virtual const NodeIDType& getTargetNodeID()const = 0;
	// EdgeType equal , as well as isSameTypeEdge function;
	virtual bool operator==(const EdgeType &e)const = 0;
	virtual bool isSameTypeEdge(const EdgeType &n) const = 0;
	virtual const EdgeLabelType&  getLabel()const = 0;
	virtual bool operator<(const EdgeType &e)const = 0;
};


template<typename _EdgeLabelType>
class EdgeVF2 :public Edge<_EdgeLabelType> {

public:
	//SOURCE edge only allows to gain it's source node id . TARGET edge is similar to SOURCE edge;
	//BOTH edge allows to get both source and target  nodes id
	//using this to avoid logical error , ofc all edges can be set to BOTH type; 
	enum NODE_RECORD_TYPE { SOURCE, TARGET, BOTH };
	typedef size_t NodeIDType;
	typedef _EdgeLabelType EdgeLabelType;
	typedef EdgeVF2<EdgeLabelType> EdgeType;
	typedef Edge<EdgeLabelType> EdgeBaseType;
private:
	NodeIDType source, target;
	NODE_RECORD_TYPE recordType;
	EdgeLabelType label = EdgeLabelType();
public:
	EdgeVF2() = default;
	~EdgeVF2() = default;
	EdgeVF2(NODE_RECORD_TYPE _recordType, const NodeIDType _source, const NodeIDType _target) :
		recordType(_recordType), source(_source), target(_target) {
	}
	EdgeVF2(NODE_RECORD_TYPE _recordType, const NodeIDType _source, const NodeIDType _target, const EdgeLabelType _label) :
		recordType(_recordType), source(_source), target(_target), label(_label) {
	}
	const NodeIDType& getSourceNodeID() const {
		if (recordType == NODE_RECORD_TYPE::TARGET) throw "this is a edge record target node!!";
		return source;
	}
	const NodeIDType& getTargetNodeID()const
	{
		if (recordType == NODE_RECORD_TYPE::SOURCE)throw "this is a edge record source node!!";
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
	bool operator<(const EdgeBaseType &e)const {
		assert(recordType != NODE_RECORD_TYPE::BOTH && "Both edge cannot be compare");
		if (recordType == NODE_RECORD_TYPE::TARGET) {
			if (target == e.getTargetNodeID()) return label < e.getLabel();
			else return target < e.getTargetNodeID();
		}
		else if (recordType == NODE_RECORD_TYPE::SOURCE) {
			if (source == e.getSourceNodeID()) return label < e.getLabel();
			else return source < e.getSourceNodeID();
		}
		else assert("error situation");
		return false;
	};
};

