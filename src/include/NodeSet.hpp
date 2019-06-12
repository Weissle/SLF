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
	size_t _size;
public:
	NodeSet() = default;
	~NodeSet() = default;;
	NodeSet(size_t need):_size(need) {
		s.reserve(calHashSuitableSize(need));
		belong = new bool[need];
		for (auto i = 0; i < need; ++i) {
			belong[i] = false;
		}
	}

	void insert(NodeIDType id) {
		if (id >= _size) {
			throw "id is too large";
		}
		if (belong[id])return;
		else {
			belong[id] = true;
			s.insert(id);
		}
	}
	void erase(NodeIDType id) {
		if (id >= _size) {
			throw "id is too large";
		}
		if (belong[id] == false)return;
		else {
			belong[id] = false;
			s.erase(id);
		}
	}
	auto find(const NodeIDType id)const {
		if (id >= _size || belong[id] == false)return s.end();
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

};


