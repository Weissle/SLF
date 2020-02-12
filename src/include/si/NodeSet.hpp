#pragma once
#include"common.h"
#include "si_marcos.h"
#include<assert.h>
#include<unordered_set>
#include<memory>
// only use bool[] or unordered_set cost much time
using namespace std;
namespace wg {
template<class _GraphType = void>
class NodeSet {
	typedef size_t NodeIDType;
	unordered_set<NodeIDType> s;
	vector<bool> belong;
	size_t _max_size, _size = 0;
public:
	NodeSet() = default;
	~NodeSet() = default;
	NodeSet(size_t need) :_max_size(need) {
		s.reserve(calHashSuitableSize(need));
		belong = move(vector<bool>(need));
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
	typedef unordered_set<NodeIDType> VUnit;
private:
	
	vector< VUnit > v;
	vector<size_t> vSize;
	vector<bool> belong;
	GraphType const* graph = nullptr;
public:

	NodeSetWithLabel(const GraphType& _graph) :graph(&_graph) {
		const auto LQinform = _graph.LQinform();
		v.resize(LQinform.size());
		vSize.resize(LQinform.size());
		for (auto it = LQinform.begin(); it != LQinform.end(); ++it) {
			v[it->first].reserve(calHashSuitableSize(it->second));
		}
		belong.resize(_graph.size());
	}
	void insert(const NodeIDType id) {
		if (belong[id] == false) {
			const auto label = graph->node(id).label();
			vSize[label]++;
			v[label].insert(id);
			belong[id] = true;
		}
		return;
	}
	void erase(const NodeIDType id) {
		if (belong[id] == true) {
			const auto label = graph->node(id).label();
			vSize[label]--;
			v[label].erase(id);
			belong[id] = false;
		}
		return;
	}
	inline bool exist(const NodeIDType id)const {
		return belong[id];
	}
	const VUnit& getSet(NodeLabelType label)const {
		return v[label];
	}
	bool operator>(const NodeSetWithLabel<GraphType>& ns)const {
		if (vSize.size() > ns.vSize.size())return true;
		LOOP(i, 0, vSize.size()) {
			if (vSize[i] > ns.vSize[i])return true;
		}
		return false;
	}
	const VUnit& operator[](const NodeLabelType label)const {
		return v[label];
	}
	size_t size(NodeLabelType label)const {
		return vSize[label];
	}

	void containerClone(NodeSetWithLabel<GraphType>& temp) {
		v = temp.v;
		for (auto& s : v) s.clear();
		vSize = vector<size_t>(temp.vSize.size());
		graph = temp.graph;
		belong = vector<bool>(temp.belong.size());
	}
	NodeSetWithLabel() = default;
};

}