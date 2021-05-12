#pragma once
#include<assert.h>
#include<si/si_marcos.h>
namespace wg {

template<typename _LT>
class SourceEdge {
	NodeIDType _source;
	_LT _label;
public:
	typedef _LT LabelType;
	SourceEdge() = default;
	SourceEdge(const NodeIDType _s, _LT _l) :_source(_s), _label(_l) {}
	const NodeIDType source()const { return _source; }
	const LabelType label()const { return _label; }
	bool operator<(const SourceEdge<_LT>& e)const {
		if (_source == e._source) return _label < e._label;
		else return _source < e._source;
	}
};

template<typename _LT>
class TargetEdge {
	NodeIDType _target;
	_LT _label;
public:
	typedef _LT LabelType;
	TargetEdge() = default;
	TargetEdge(const NodeIDType _t, _LT _l) :_target(_t), _label(_l) {}
	const LabelType label()const { return _label; }
	const NodeIDType target()const { return _target; }
	bool operator<(const TargetEdge<_LT>& e) const {
		if (_target == e._target) return _label < e._label;
		else return _target < e._target;
	}
};

}
