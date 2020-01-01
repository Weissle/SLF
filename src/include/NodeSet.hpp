#pragma once
#include"common.h"
#include "si_marcos.h"
#include<assert.h>
#include<unordered_set>
#include<memory>
// only use bool[] or unordered_set cost much time
using namespace std;

class NodeSet {
	typedef size_t NodeIDType;
	unordered_set<NodeIDType> s;
	shared_ptr<bool[]> belong = shared_ptr<bool[]>(nullptr);
	size_t _max_size, _size = 0;
public:
	NodeSet() = default;
	~NodeSet() = default;
	NodeSet(size_t need) :_max_size(need) {
		s.reserve(calHashSuitableSize(need));
		belong = move(shared_ptr<bool[]>(new bool[need]));
		for (auto i = 0; i < need; ++i) {
			belong[i] = false;

		}
	}
	void insert(NodeIDType id) {
		if (id >= _max_size) {
			throw "id is too large";
		}
		if (belong[id])return;
		else {
			belong[id] = true;
			s.insert(id);
			++_size;
		}
	}
	void erase(NodeIDType id) {
		if (id >= _max_size) {
			throw "id is too large";
		}
		if (belong[id] == false)return;
		else {
			belong[id] = false;
			s.erase(id);
			--_size;
		}
	}
	auto find(const NodeIDType id)const {
		if (id >= _max_size || belong[id] == false)return s.end();
		else return s.find(id);
	}
	auto end()const {

		return s.end();
	}
	auto begin()const {

		return s.begin();
	}
	auto getSet()const {
		return s;
	}
	auto size()const {
		return _size;
	}

};
template<class _GraphType>
class NodeSetWithLabel {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
private:
	//	vector< unordered_map<NodeIDType, bool> > v;
	vector< unordered_set<NodeIDType> > v;
	vector<size_t> vSize;
	GraphType const* graph = nullptr;
public:
	NodeSetWithLabel(const GraphType& _graph) :graph(&_graph) {
		const auto LQinform = _graph.getLQinform();
		
		v.resize(LQinform.size());
		vSize.resize(LQinform.size());
		for (auto it = LQinform.begin(); it != LQinform.end(); ++it) {
			v[it->first].reserve(it->second);
		}
		
	}
	NodeSetWithLabel(const NodeSetWithLabel<GraphType>& temp) {
		v = temp.v;
		vSize = temp.vSize;
		graph = temp.graph;
	}
	void insert(const NodeIDType id) {
		const auto label = graph->getNode(id).getLabel();
		if (NOT_IN_SET(v[label], id)) {
			vSize[label]++;
			v[label].insert(id);
		}
		
		return;
	}
	void erase(const NodeIDType id) {
		const auto label = graph->getNode(id).getLabel();
		if (IN_SET(v[label], id)) {
			vSize[label]--;
			v[label].erase(id);
		}
		return;
	}
	bool contain(const NodeIDType id)const {
		const auto label = graph->getNode(id).getLabel();
		return IN_SET(v[label], id);
	}
	const unordered_set<NodeIDType>& getSet(NodeLabelType label) {
		return v[label];
	}
	bool operator<=(const NodeSetWithLabel<GraphType>& ns)const {
		if (vSize.size() > ns.vSize.size())return false;
		LOOP(i, 0, min(vSize.size(), ns.vSize.size())) {
			if (vSize[i] > ns.vSize[i])return false;
		}
		return true;
	}
	bool operator>=(const NodeSetWithLabel<GraphType>& ns)const {
		return ns <= *this;
	}
	bool operator<(const NodeSetWithLabel<GraphType>& ns)const {
		return !(ns >= *this);
	}
	bool operator>(const NodeSetWithLabel<GraphType>& ns)const {
		return !(ns <= *this);
	}
	const unordered_set<NodeIDType>& operator[](const NodeLabelType label)const {
		return v[label];
	}
	size_t size(NodeLabelType label)const {
		return vSize[label];
	}

	NodeSetWithLabel() = default;
//	~NodeSetWithLabel() = default;
private:

};


