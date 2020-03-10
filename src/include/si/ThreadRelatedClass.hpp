#pragma once;
#include<utility>
#include<vector>
#include<mutex>
#include<assert.h>
using namespace std;
namespace wg {
template<class _Ty>
class vector_mutex {
	vector<_Ty> q;
public:
	std::mutex m;
	vector_mutex() = default;
	void push_back(_Ty t) {
		q.push_back(t);
	}
	_Ty pop() {
		auto t = q.back();
		q.pop_back();
		return t;
	}
	bool empty() {
		return q.empty();
	}
	size_t size() {
		return q.size();
	}
	_Ty& operator[](size_t t) {
		return q[t];
	}
};

}