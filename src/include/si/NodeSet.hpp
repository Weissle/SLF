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
template<class GraphType>
class NodeSetSimple {
protected:
	GraphType const* graph = nullptr;
	vector<bool> belong;

public:
	NodeSetSimple() = default;
	NodeSetSimple(const GraphType& _g) :graph(&_g) {
		belong.resize(_g.size());
	}
	inline void insert(const NodeIDType id) {
		belong[id] = true;
	}
	inline void erase(const NodeIDType id) {
		belong[id] = false;
	}
	inline bool exist(const NodeIDType id)const {
		return belong[id];
	}
};
template<class GraphType>
class NodeSetWithLabel :NodeSetSimple<GraphType> {
public:
	typedef size_t NodeLabelType;
	typedef vector<NodeIDType> Nodes;
private:
	vector<size_t> place;
	vector<Nodes> v;
public:

	NodeSetWithLabel(const GraphType& _graph) :NodeSetSimple<GraphType>(_graph) {
		const auto labelsNum = _graph.labelNum();
		place.resize(_graph.size(), NO_MAP);
		v.resize(_graph.maxLabel() + 1);
		for (auto it = labelsNum.begin(); it != labelsNum.end(); ++it) {
			v[it->first].reserve(it->second + 1);
		}
	}
	void insert(const NodeIDType id) {

		if (place[id] == NO_MAP) {
			const auto label = graph->node(id).label();
			place[id] = v[label].size();
			v[label].push_back(id);
		}
		return;
	}
	void erase(const NodeIDType id) {
		if (place[id] != NO_MAP) {
			const auto label = graph->node(id).label();
			auto& vl = v[label];
			size_t swapPlace = vl.size() - 1;
			size_t swapID = vl[swapPlace];
			size_t id_place = place[id];
			swap(vl[id_place], vl[swapPlace]);
			swap(place[id], place[swapID]);
			place[id] = NO_MAP;
			vl.pop_back();
		}
		return;
	}
	inline bool exist(const NodeIDType id)const {
		return place[id] != NO_MAP;
	}
	const Nodes& getSet(NodeLabelType label)const {
		//		if (label >= v.size()) return move(Nodes());
		return v[label];
	}
	size_t size(NodeLabelType label)const {
		return v[label].size();
	}

	NodeSetWithLabel() = default;
};

template<class GraphType>
class NodeSetWithDepth :NodeSetSimple<GraphType> {
public:
	typedef size_t NodeLabelType;
	typedef vector<NodeIDType> Nodes;
private:
	vector<size_t> place, depthSpace;
	Nodes p;
	size_t nowDepth = 0;

	inline size_t& start_place(const size_t depth){
		return depthSpace[depth << 1];
	}
	inline size_t& mid_place(const size_t depth){
		return depthSpace[(depth << 1)+1];
	}
	inline size_t& end_place(const size_t depth){
		return depthSpace[(depth + 1) << 1];
	}
public:
	inline void double_swap(size_t id1, size_t id2) {
		auto& p1 = place[id1], & p2 = place[id2];
		swap(p[p1], p[p2]);
		swap(p1, p2);
	}
	NodeSetWithDepth(const GraphType& _graph, size_t depth) :NodeSetSimple<GraphType>(_graph) {
		const auto labelsNum = _graph.labelNum();
		place.resize(_graph.size(), NO_MAP);
		p.reserve(_graph.size() + 1);
		depthSpace.resize(2 * depth + 3, 0);
	}
	void prepare(const size_t depth) {
		assert(depthSpace[(depth << 1) + 1] == depthSpace[(depth << 1) + 2]);
		assert(depthSpace[(depth << 1) + 1] == 0);
		mid_place(depth) = end_place(depth) = start_place(depth);
		nowDepth++;
	}
	void insert(const NodeIDType id, const size_t depth) {
		if (id == 468468) {
			int a = 0;
		}
		if (belong[id] == false) {
			belong[id] = true;
			if (place[id] == NO_MAP) {
				assert(nowDepth == depth);
				assert(depthSpace[(depth << 1) + 1] == depthSpace[(depth << 1) + 2]);
				nowDepth = depth;
				place[id] = p.size();
				p.push_back(id);
				mid_place(depth)++;
				end_place(depth)++;
			}
			else {
		//		assert(place[id] == place[p[depthSpace[(depth << 1) + 1]]]);
		//		double_swap(id, p[depthSpace[(depth << 1) + 1]]);
				mid_place(depth)++;
			}

		}
		return;
	}
	void erase(const NodeIDType id, size_t depth) {
		if (id == 684684) {
			int a = 0;
		}
		assert(depth <= nowDepth);
		if (belong[id] == true) {
			belong[id] = false;
			mid_place(depth)--;
			double_swap(id, p[mid_place(depth)]);
		}
		return;
	}
	void pop(size_t depth) {
		assert(depth = nowDepth);
		assert(depthSpace[(depth << 1) + 1] == depthSpace[(depth << 1) + 2]);
		assert(depthSpace[(depth << 1) + 1] == p.size());
		nowDepth--;
		auto end = end_place(depth);
		auto start = start_place(depth);
		LOOP(i, start, end) {
			const auto& id = p[i];
			belong[id] = false;
			place[id] = NO_MAP;
		}
		LOOP(i, start, end) {
			p.pop_back();
		}

		mid_place(depth) = end_place(depth) = 0;
	}
	inline bool exist(const NodeIDType id)const {
		return belong[id];
	}
	void getSet(size_t depth,const NodeIDType *&begin,const NodeIDType *&mid)const {
		assert(depth <= nowDepth);
		begin = p.data() + depthSpace[depth << 1];
		mid = p.data() + depthSpace[(depth << 1) + 1];
	}

	NodeSetWithDepth() = default;
};

}