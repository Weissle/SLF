#pragma once;
#include<utility>
#include<vector>
#include<mutex>
#include<common.h>
#include<assert.h>
using namespace std;
namespace wg {
class SearchTree {
protected:
	typedef size_t NodeIDType;
	typedef pair<NodeIDType, NodeIDType> MapPair;
	vector<vector<MapPair>> tree;
	size_t maxDepth;
public:
	SearchTree() = default;
	SearchTree(size_t _size) :maxDepth(_size) { tree.resize(_size); }
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
		return answer;
	}
	void setTree(size_t depth, const vector<MapPair>& v) {
		assert(depth < maxDepth && "depth >= maxDepth");
		tree[depth] = v;
	}
};
class SearchTreeThread:SearchTree {
	size_t minDepth = 0;
	mutex m;
public:
	SearchTreeThread() = default;
	SearchTreeThread(size_t _size) :SearchTree(_size) {}
	bool empty(size_t depth) {
		LOCK_TO_END(m);
		return SearchTree::empty(depth);
	}
	size_t size(size_t depth){
		LOCK_TO_END(m);
		return SearchTree::size(depth);
	}
	MapPair pop(size_t depth) {
		LOCK_TO_END(m);
		assert(depth < maxDepth && "depth >= maxDepth");
		auto answer = tree[depth].back();
		tree[depth].pop_back();
		if (depth == minDepth && tree[depth].empty()) ++minDepth;
		return answer;
	}
	void workDistribute(SearchTreeThread& s) {
		m.lock();
		vector<MapPair>& pairs = tree[minDepth];
		assert(pairs.size() != 0);
		auto it = pairs.begin() + pairs.size() % 2 + pairs.size() / 2;
		vector<MapPair> pairsToGive(pairs.begin(), it);
		auto sum = pairs.size(), sp = pairsToGive.size();
		pairs.assign(it, pairs.end());
		assert(sum == sp + pairs.size());
		while (minDepth<=maxDepth && tree[minDepth].empty() == true) {
			minDepth++;
		}
		m.unlock();
		s.setTree(minDepth, move(pairsToGive), true);
	} 
	void setTree(size_t depth, const vector<MapPair>& v, bool resetOther=false) {
		LOCK_TO_END(m);
		assert(depth < maxDepth && "depth >= maxDepth");
		if (resetOther) {
			minDepth = depth;
			LOOP(i, 0, maxDepth)tree[depth].clear();
		}
		tree[depth] = v;
	}
	pair<size_t, size_t> minDepth_and_restPair(){
		LOCK_TO_END(m);
		if (tree[minDepth].size() != 0) {
			return pair<size_t, size_t>(NO_MAP, 0);
		}
		return pair<size_t, size_t>(minDepth, tree[minDepth].size());
	}
};

}