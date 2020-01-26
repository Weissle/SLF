#pragma once
#include<map>
#include<vector>
#include<unordered_map>
#include<typeinfo>
#include"common.h"
using namespace std;
/*
In this subgraph isomorphism algorithm ,node id's type is size_t
So in case of your graph node id's type is not size_t, such as string, IndexTurner can turn it to unique id (of course type is size_t).
NameType is your node id's type
IndexType should similar to std::map or std::unordered_map
If IndexType is std::map , NameType should define operator < function;
If IndexType is unordered_map ,you should define a hash function for NameType .
Main function is operator [] and () ,
operator[] is used in graph output, it can turn size_t to NameType
operator() is used in graph input, it can turn NameType to size_t

*/
namespace wg {
template<typename _NameType, typename _IndexType = unordered_map<_NameType, size_t> >
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
		if (typeid(IndexType) == typeid(unordered_map<_NameType, int>))index.reserve(calHashSuitableSize(s));
	}
	~IndexTurner() = default;
	void clear() {
		_s = 0;
		index.clear();
	}
	size_t size() { return _s; }
	//from graph id to user id
	NameType operator[](const size_t id)const {
		if (id > _s) throw "id>size()";
		return BIndex[id];
	}
	NameType userID(const size_t id)const {
		this->operator[](id);
	}
	//from user id to graph id
	size_t operator()(const NameType& name) {
		auto temp = index.find(name);
		if (temp == index.end()) {
			index[name] = _s;
			BIndex.push_back(name);
			++_s;
			return _s - 1;

		}
		else {
			return temp->second;
		}
	}
	size_t graphID(const NameType& name) {
		return this->operator()(name);
	}
	auto begin() {
		return index.begin();
	}
	auto end() {
		return index.end();
	}
	bool exist(const NameType& name) {
		return IN_SET(index, name);
	}
};
}