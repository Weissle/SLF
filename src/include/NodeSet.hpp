#pragma once
#include"common.h"
#include "si_marcos.h"
#include<assert.h>
#include<unordered_set>
#include<memory>
// only use bool[] or unordered_set cost much time
using namespace std;

template<class _GraphType = void>
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
		belong = move(shared_ptr<bool[]>(new bool[need]()));
	}
	NodeSet(const _GraphType& _g) :NodeSet(_g.size()) {}
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
	auto exist(const NodeIDType id)const {
		if (id >= _max_size || belong[id] == false)return false;
		else return true;
	}
	auto end()const {
		return s.end();
	}
	bool operator>(const NodeSet& temp)const {
		return _size > temp._size;
	}
	auto begin()const {

		return s.begin();
	}
	const unordered_set<NodeIDType>& getSet()const {
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
	unique_ptr<bool[]> belong;

	GraphType const* graph = nullptr;
public:
	size_t all_size = 0;
	NodeSetWithLabel(const GraphType& _graph) :graph(&_graph) {
		const auto LQinform = _graph.getLQinform();
		v.resize(LQinform.size());
		vSize.resize(LQinform.size());
		for (auto it = LQinform.begin(); it != LQinform.end(); ++it) {
			v[it->first].reserve(calHashSuitableSize(it->second));
		}
		belong = unique_ptr<bool[]>(new bool[graph->size()]());
	}
	NodeSetWithLabel(const NodeSetWithLabel<GraphType>& temp) {
		v = temp.v;
		vSize = temp.vSize;
		graph = temp.graph;
		belong = shared_ptr<bool[]>(new bool[graph->size()]());
	}
	void insert(const NodeIDType id) {
		if (belong[id] == false) {
			const auto label = graph->getNode(id).getLabel();
			vSize[label]++;
			v[label].insert(id);
			belong[id] = true;
			++all_size;
		}
		return;
	}
	void erase(const NodeIDType id) {
		if (belong[id] == true) {
			const auto label = graph->getNode(id).getLabel();
			vSize[label]--;
			v[label].erase(id);
			--all_size;
			belong[id] = false;
		}
		return;
	}
	bool exist(const NodeIDType id)const {
		return belong[id];
		/*	const auto label = graph->getNode(id).getLabel();
			return IN_SET(v[label], id);*/
	}
	const unordered_set<NodeIDType>& getSet(NodeLabelType label)const {
		return v[label];
	}
	bool operator>(const NodeSetWithLabel<GraphType>& ns)const {
		if (all_size > ns.all_size)return true;
		if (vSize.size() > ns.vSize.size())return true;
		LOOP(i, 0, vSize.size()) {
			if (vSize[i] > ns.vSize[i])return true;
		}
		return false;
	}
	const unordered_set<NodeIDType>& operator[](const NodeLabelType label)const {
		return v[label];
	}
	size_t size(NodeLabelType label)const {
		return vSize[label];
	}

	void operator=(NodeSetWithLabel<GraphType>& temp) {
		v = temp.v;
		vSize = temp.vSize;
		belong = std::move(temp.belong);
		graph = temp.graph;
	}
	void emptyClone(NodeSetWithLabel<GraphType>& temp) {
		v = temp.v;
		for (auto& s : v) s.clear();
		vSize = temp.vSize;
		for (auto& i : vSize) i = 0;
		graph = temp.graph;
		belong = std::move(unique_ptr<bool[]>(new bool[graph->size()]()));
	}
	NodeSetWithLabel() = default;
};