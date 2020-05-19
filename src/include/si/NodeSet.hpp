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
	vector<size_t> place;
	vector<vector<NodeIDType>> p;

public:

	NodeSetWithDepth(const size_t& _size, size_t depth) :NodeSetSimple(_size) {
		place.resize(_size, NO_MAP);
		p.resize(depth + 1);
	}
	void prepare(const size_t depth) {
		return;
	}
	void insert(const NodeIDType id, const size_t depth) {
		if (belong[id] == false) {
			belong[id] = true;
			assert(place[id] == NO_MAP);
			place[id] = p[depth].size();
			p[depth].push_back(id);
		}
		return;
	}
	void erase(const NodeIDType id, size_t depth) {
		if (belong[id] == true) {
			auto& p_depth = p[depth];
			belong[id] = false;
			place[p_depth.back()] = place[id];
			p_depth[place[id]] = p_depth.back();
			p_depth.pop_back();
			place[id] = NO_MAP;
		}
		return;
	}
	void pop(size_t depth) {
		static mutex m;

		for(const auto &id:p[depth]) {
			belong[id] = false;
			place[id] = NO_MAP;
		}
		p[depth].clear();

	}

	void getSet(size_t depth, const NodeIDType*& begin, const NodeIDType*& end)const {
		begin = p[depth].data();
		end = p[depth].data() + p[depth].size();
	}

	NodeSetWithDepth() = default;
};

}