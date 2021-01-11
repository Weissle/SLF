#pragma once
#include"Edge.hpp"
#include"si/si_marcos.h"
#include<algorithm>
#include<vector>
using namespace std;
namespace wg {
template<typename _EdgeLabelType>
class Node {
public:
	typedef _EdgeLabelType EdgeLabelType;
	typedef Node<_EdgeLabelType> NodeType;
	typedef size_t NodeLabelType;
private:

	NodeIDType _id;
	NodeLabelType _label;
	vector<SourceEdge<EdgeLabelType>> _inEdges;
	vector<TargetEdge<EdgeLabelType>> _outEdges;
	
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
		const TargetEdge<EdgeLabelType> tar(to,elabel);
		return binary_search(_outEdges.begin(),_outEdges.end(),tar);
	}
	/*
	bool existSameTypeEdgeFromNode(const NodeIDType from, const EdgeLabelType &elabel)const {
		//return findEdge(_inEdges, EdgeType(EDGE_RECORD_TYPE::SOURCE, from, _id, elabel));
	}*/
	void reserve(const size_t s) {
		_inEdges.reserve(s);
		_outEdges.reserve(s);
	}
	inline const auto& outEdges() const { return _outEdges; }
	inline const auto& inEdges() const { return _inEdges; }
	inline size_t outEdgesNum() const { return _outEdges.size(); }
	inline size_t inEdgesNum() const { return _inEdges.size(); }
	inline size_t nodeIdHash()const { return hash<NodeIDType>()(this->_id); }
	inline void addInEdge(const NodeIDType id, EdgeLabelType l) { _inEdges.emplace_back(id, l); }
	inline void addOutEdge(const NodeIDType id, EdgeLabelType l) { _outEdges.emplace_back(id, l); }
	void NodeBuildFinish() {
		sort(_inEdges.begin(), _inEdges.end());
		sort(_outEdges.begin(), _outEdges.end());
	}
};
}
