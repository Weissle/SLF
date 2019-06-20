#pragma once
#include<map>
#include<vector>
#include<unordered_map>
#include<typeinfo>
#include"common.h"
using namespace std;
template<typename _NameType,typename _IndexType=unordered_map<_NameType,size_t> >
class IndexTurner {
public:
	typedef _NameType NameType;
	typedef _IndexType IndexType;
private:
	vector<NameType> BIndex;
	IndexType index;
	size_t _s = 0;
public:
	IndexTurner() = default;
	IndexTurner(const size_t  s) {
		BIndex.reserve(s);
		if ( typeid(IndexType) == typeid( unordered_map<_NameType, int> ) )index.reserve(calHashSuitableSize(s));
	}
	~IndexTurner() = default;
	size_t size() { return _s };
	//from graph id to user id
	NameType operator[](const size_t id)const {
		if (id > _s) throw "id>size()";
		return BIndex[id];
	}
	NameType userID(const size_t id)const {
		this->[](id);
	}
	//from user id to graph id
	size_t operator()(const NameType& name) {
		auto temp = index.find(name);
		if (temp == index.end()) {
			index[name] = _s;
			BIndex.push_back(name);
			++_s;
			return _s-1;
		
		}
		else {
			return temp->second;
		}
	}
	size_t graphID(const NameType& name) {
		return this->(name);
	}
	auto begin() {
		return index.begin();
	}
	auto end() {
		return index.end();
	}

};