#pragma once;
#include<utility>
#include<vector>
#include<mutex>
#include<assert.h>
using namespace std;
namespace wg {
class SearchTree {
	typedef size_t NodeIDType;
	typedef pair<NodeIDType, NodeIDType> MapPair;
	vector<vector<MapPair>> tree;
	size_t maxDepth, minDepth;
public:
	SearchTree() = default;
	SearchTree(size_t _size) :maxDepth(_size), minDepth(0) { tree.resize(_size); }
	bool empty(size_t depth)  const {
		assert(depth < maxDepth && "depth >= maxDepth");
		return tree[depth].empty();
	}
	size_t size(size_t depth) const {
		assert(depth < maxDepth && "depth >= maxDepth");
		return tree[depth].size();
	}
	MapPair pop(size_t depth) {
		assert(depth < maxDepth && "depth >= maxDepth");
		auto answer = tree[depth].back();
		tree[depth].pop_back();
		if (depth == minDepth && tree[depth].empty()) ++minDepth;
		return answer;
	}
	void workDistribute(SearchTree& s) {
		vector<MapPair>& pairs = tree[minDepth];
		assert(pairs.size() != 0);
		s.minDepth = minDepth;
		LOOP(i, 0, maxDepth)  s.tree[i].clear();
		auto it = pairs.begin() + pairs.size() % 2 + pairs.size() / 2;
		s.tree[minDepth].assign(pairs.begin(), it);
		tree[minDepth].assign(it, pairs.end());
	}
	void setTree(size_t depth, const vector<MapPair>& v, bool resetOther = false) {
		assert(false);
		assert(depth < maxDepth && "depth >= maxDepth");
		tree[depth] = v;
		if (resetOther) {
			minDepth = depth;
			LOOP(i, 0, maxDepth) if (i != depth) tree[depth].clear();
		}
	}
	pair<size_t, size_t> minDepth_and_restPair()const {
		return pair<size_t, size_t>(minDepth, tree[minDepth].size());
	}
};

class vector_mutex {
	mutex m;
	vector<size_t> q;
#define LOCK_TO_END lock_guard<mutex> lg(m)
public:
	vector_mutex() = default;
	void push_back(size_t t) {
		LOCK_TO_END;
		q.push_back(t);
	}
	size_t pop() {
		LOCK_TO_END;
		auto t = q.front();
		q.pop_back();
		return t;
	}
	bool empty() {
		LOCK_TO_END;
		return q.empty();
	}
	size_t size() {
		LOCK_TO_END;
		return q.size();
	}
	size_t operator[](size_t t) {
		LOCK_TO_END;
		return q[t];
	}
};

}