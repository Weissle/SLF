#pragma once
#include<assert.h>
#include<si/si_marcos.h>
namespace wg {
//SOURCE edge only allows to gain it's source node id . TARGET edge is similar to SOURCE edge;
//BOTH edge allows to get both source and target  nodes id
enum EDGE_RECORD_TYPE { SOURCE, TARGET, BOTH };
template<typename _EdgeLabelType>
class EdgeLogic {
public:
	typedef _EdgeLabelType EdgeLabelType;
	typedef EdgeLogic<EdgeLabelType> EdgeType;
private:
	NodeIDType _source, _target;
	EDGE_RECORD_TYPE recordType;
	EdgeLabelType _label;
	EdgeLogic() = default;
public:
	~EdgeLogic() = default;
	EdgeLogic(EDGE_RECORD_TYPE _recordType, const NodeIDType _s, const NodeIDType _t, const EdgeLabelType _l = EdgeLabelType()) :
		recordType(_recordType), _source(_s), _target(_t), _label(_l) {
	}

	const NodeIDType& source() const {
		assert(recordType != EDGE_RECORD_TYPE::TARGET && "this is a edge record target node!!");
		return _source;
	}
	const NodeIDType& target()const
	{
		assert(recordType != EDGE_RECORD_TYPE::SOURCE && "this is a edge record source node!!");
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
		assert(recordType != EDGE_RECORD_TYPE::BOTH && "Both edge cannot be compare");
		if (recordType == EDGE_RECORD_TYPE::TARGET) {
			if (_target == e.target()) return _label < e.label();
			else return _target < e.target();
		}
		else if (recordType == EDGE_RECORD_TYPE::SOURCE) {
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

template<typename _EdgeLabelType>
class EdgeSimple {
public:
	typedef _EdgeLabelType EdgeLabelType;
	typedef EdgeSimple<EdgeLabelType> EdgeType;
private:
	NodeIDType _source, _target;
	EdgeLabelType _label;
	EdgeSimple() = default;
public:
	~EdgeSimple() = default;
	EdgeSimple(EDGE_RECORD_TYPE _recordType, const NodeIDType _s, const NodeIDType _t, const EdgeLabelType _l = EdgeLabelType()) :_label(_l), _source(_s), _target(_t)
	{}

	inline const NodeIDType& source() const { return _source; }
	inline const NodeIDType& target()const { return _target; }
	inline const EdgeLabelType& label()const { return _label; }
	inline bool isSameTypeEdge(const EdgeType& n) const {return _label == n.label();}
	bool operator<(const EdgeType& e)const {
		if (_source == e._source) {
			if (_target == e._target) {
				return _label < e._label;
			}
			return _target < e._target;
		}
		return _source < e._source;
	};
	inline bool operator==(const EdgeType& e)const {
		return (_source == e._source && _label == e._label && _target == e._target);
	}

};

}