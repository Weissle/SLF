#pragma once
#include<typeinfo>
#include<assert.h>

namespace wg {
template<typename _EdgeLabelType>
class Edge {

public:
	//SOURCE edge only allows to gain it's source node id . TARGET edge is similar to SOURCE edge;
	//BOTH edge allows to get both source and target  nodes id
	enum NODE_RECORD_TYPE { SOURCE, TARGET, BOTH };
	typedef size_t NodeIDType;
	typedef _EdgeLabelType EdgeLabelType;
	typedef Edge<EdgeLabelType> EdgeType;
private:
	NodeIDType _source, _target;
	NODE_RECORD_TYPE recordType;
	EdgeLabelType _label;
public:
	Edge() = default;
	~Edge() = default;
	Edge(NODE_RECORD_TYPE _recordType, const NodeIDType _s, const NodeIDType _t, const EdgeLabelType _l = EdgeLabelType()) :
		recordType(_recordType), _source(_s), _target(_t), _label(_l) {
	}

	const NodeIDType& source() const {
		assert(recordType != NODE_RECORD_TYPE::TARGET&&  "this is a edge record target node!!");
		return _source;
	}
	const NodeIDType& target()const
	{
		assert (recordType != NODE_RECORD_TYPE::SOURCE && "this is a edge record source node!!");
		return _target;
	}
	const EdgeLabelType& label()const
	{
		return _label;
	}
	bool isSameTypeEdge(const EdgeType& n) const {
		return _label == n.label();
	}

	bool operator<(const EdgeType& e)const {
		assert(recordType != NODE_RECORD_TYPE::BOTH && "Both edge cannot be compare");
		if (recordType == NODE_RECORD_TYPE::TARGET) {
			if (_target == e.target()) return _label < e.label();
			else return _target < e.target();
		}
		else if (recordType == NODE_RECORD_TYPE::SOURCE) {
			if (_source == e.source()) return _label < e.label();
			else return _source < e.source();
		}
		else throw "error situation";
		return false;
	};
	bool operator==(const EdgeType& e)const {
		if (this->recordType != e.recordType || this->_label != e._label) return false;
		if (this->_source != e._source || this->_target != e._target)return false;
		return true;
	}
};

}