#pragma once
#include"Edge.hpp"
#include"si/si_marcos.h"
#include<functional>
#include<algorithm>
#include<vector>
using namespace std;
namespace wg {
template<typename _EdgeType>
class Node {
public:
	typedef size_t NodeLabelType;
	typedef _EdgeType EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

private:
	NodeIDType _id;
	NodeLabelType _label;
	vector<EdgeType> _inEdges, _outEdges;

	bool findEdge(const vector<EdgeType>& edges, const EdgeType&& edge) const {
		return binary_search(edges.begin(), edges.end(), edge);
	}
	inline int _cmp_edge_target(const EdgeType& e, const NodeIDType id, const EdgeLabelType& label)const {
		if (e.target() == id) {
			if (e.label() == label)return -1;
			return e.label() < label;
		}
		else {
			if (e.target() == id)return -1;
			else return e.target() < id;
		}
	}
	Node() = default;
public:
	~Node() = default;
	Node(const NodeIDType _i, const NodeLabelType _l = NodeLabelType()) :_id(_i), _label(_l) {}
	inline const NodeIDType id()const { return _id; }
	inline const NodeLabelType& label()const { return _label; }
	inline void setLabel(const NodeLabelType _l) { this->_label = _l; }
	inline bool isSameType(const NodeType& n)const { return _label == n.label(); }
	inline bool operator>=(const NodeType& n) const { return ((_inEdges.size() >= n.inEdgesNum()) && (_outEdges.size() >= n.outEdgesNum())); }
	bool existSameTypeEdgeToNode(const NodeIDType to, const EdgeLabelType& elabel)const {
		size_t begin = 0, end = _outEdges.size();
		while (begin < end) {
			const auto mid = (begin + end) / 2;
			const auto& ele = _outEdges[mid];
			if (_cmp_edge_target(ele, to, elabel) == -1) return true;
			else if (_cmp_edge_target(ele, to, elabel) > 0) { begin = mid + 1; }
			else end = mid;
		}
		return false;
	}
	/*
	bool existSameTypeEdgeFromNode(const NodeIDType from, const EdgeLabelType &elabel)const {
		//return findEdge(_inEdges, EdgeType(EDGE_RECORD_TYPE::SOURCE, from, _id, elabel));
	}*/
	void reserve(const size_t s) {
		_inEdges.reserve(s);
		_outEdges.reserve(s);
	}
	inline const vector<EdgeType>& outEdges() const { return _outEdges; }
	inline const vector<EdgeType>& inEdges() const { return _inEdges; }
	inline size_t outEdgesNum() const { return _outEdges.size(); }
	inline size_t inEdgesNum() const { return _inEdges.size(); }
	inline size_t nodeIdHash()const { return hash<NodeIDType>()(this->_id); }
	inline void addInEdge(const EdgeType& e) { _inEdges.push_back(e); }
	inline void addOutEdge(const EdgeType& e) { _outEdges.push_back(e); }
	void NodeBuildFinish() {
		_inEdges.shrink_to_fit();
		_outEdges.shrink_to_fit();
		sort(_inEdges.begin(), _inEdges.end());
		sort(_outEdges.begin(), _outEdges.end());
	}
};
}