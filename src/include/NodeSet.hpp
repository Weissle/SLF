#pragma once
#include"common.h"
#include<assert.h>
#include<unordered_set>
// only use bool[] or unordered_set cost much time
using namespace std;

class NodeSet {
	typedef size_t NodeIDType;
	unordered_set<NodeIDType> s;
	bool *belong;
	size_t _max_size, _size = 0;
public:
	NodeSet() = default;
	~NodeSet() = default;
	NodeSet(size_t need):_max_size(need) {
		s.reserve(calHashSuitableSize(need));
		belong = new bool[need];
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
	auto max_size()const {
		return _max_size;
	}
	auto size()const {
		return _size;
	}

};


