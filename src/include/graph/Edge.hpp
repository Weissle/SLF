#pragma once
#include<assert.h>
#include<si/si_marcos.h>
namespace wg {

/*
 * If use std::pair<NodeIDType,EdgeLabelType>, maybe an edge (A->B) will be regarded as (B->A) because of bug code. 
 * To avoid logic error, I use SourceEdge and TargetEdge.
 *
 */
template<typename EdgeLabel>
class SourceEdge {
	NodeIDType _source;
	EdgeLabel _label;
public:
	SourceEdge() = default;
	SourceEdge(const NodeIDType _s, EdgeLabel _l) :_source(_s), _label(_l) {}
	const NodeIDType source()const { return _source; }
	const EdgeLabel label()const { return _label; }
	bool operator<(const SourceEdge<EdgeLabel>& e)const {
		if (_source == e._source) return _label < e._label;
		else return _source < e._source;
	}
	bool operator==(const SourceEdge<EdgeLabel>& e)const {
		return e._source == _source && _label == e._label ;
	}
};

template<typename EdgeLabel>
class TargetEdge {
	NodeIDType _target;
	EdgeLabel _label;
public:
	TargetEdge() = default;
	TargetEdge(const NodeIDType _t, EdgeLabel _l) :_target(_t), _label(_l) {}
	const EdgeLabel label()const { return _label; }
	const NodeIDType target()const { return _target; }
	bool operator<(const TargetEdge<EdgeLabel>& e) const {
		if (_target == e._target) return _label < e._label;
		else return _target < e._target;
	}
	bool operator==(const TargetEdge<EdgeLabel>& e)const {
		return e._target == _target && _label == e._label ;
	}
};

}
