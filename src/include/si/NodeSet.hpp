#pragma once
#include"common.h"
#include "si_marcos.h"
#include<assert.h>
#include<unordered_set>
#include<memory>
#include"ThreadRelatedClass.hpp"

using namespace std;
namespace wg {

class NodeSetSimple {
protected:
	vector<bool> belong;
public:
	NodeSetSimple() = default;
	NodeSetSimple(const size_t& _size) :belong(_size) {
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

class NodeSetWithDepth :public NodeSetSimple {
private:
	vector<size_t> place, depthSpace;
	vector<NodeIDType> p;
	size_t nowDepth = 0;

	inline size_t& start_place(const size_t depth) {
		return depthSpace[depth << 1];
	}
	inline size_t& mid_place(const size_t depth) {
		return depthSpace[(depth << 1) + 1];
	}
	inline size_t& end_place(const size_t depth) {
		return depthSpace[(depth + 1) << 1];
	}
public:
	inline void double_swap(size_t id1, size_t id2) {
		auto& p1 = place[id1], & p2 = place[id2];
		swap(p[p1], p[p2]);
		swap(p1, p2);
	}
	NodeSetWithDepth(const size_t& _size, size_t depth) :NodeSetSimple(_size) {
		place.resize(_size, NO_MAP);
		p.reserve(_size + 1);
		depthSpace.resize(2 * depth + 3, 0);
	}
	void prepare(const size_t depth) {
		assert(depthSpace[(depth << 1) + 1] == depthSpace[(depth << 1) + 2]);
		assert(depthSpace[(depth << 1) + 1] == 0);
		mid_place(depth) = end_place(depth) = start_place(depth);
		nowDepth++;
	}
	void insert(const NodeIDType id, const size_t depth) {
		if (belong[id] == false) {
			belong[id] = true;
			if (place[id] == NO_MAP) {
				assert(nowDepth == depth);
				assert(depthSpace[(depth << 1) + 1] == depthSpace[(depth << 1) + 2]);
				place[id] = p.size();
				p.push_back(id);
				mid_place(depth)++;
				end_place(depth)++;
			}
			else {
				auto &mid_p = mid_place(depth);
				swap(p[place[id]], p[mid_p]);
				place[p[mid_p]] = mid_p;

				mid_p++;
			}

		}
		return;
	}
	void erase(const NodeIDType id, size_t depth) {
		assert(depth <= nowDepth);
		if (belong[id] == true) {
			belong[id] = false;
			auto& mid_p = mid_place(depth);
			mid_p--;
			place[p[mid_p]] = place[id];
			swap(p[mid_p], p[place[id]]);
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

		mid_place(depth) = 0;
		end_place(depth) = 0;
	}

	void getSet(size_t depth, const NodeIDType*& begin, const NodeIDType*& mid)const {
		assert(depth <= nowDepth);
		begin = p.data() + depthSpace[depth << 1];
		mid = p.data() + depthSpace[(depth << 1) + 1];
	}

	NodeSetWithDepth() = default;
};

}