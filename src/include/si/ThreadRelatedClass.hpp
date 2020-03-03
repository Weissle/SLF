#pragma once;
#include<utility>
#include<vector>
#include<mutex>
#include<assert.h>
using namespace std;
namespace wg {
class vector_mutex {
	vector<size_t> q;
public:
	std::mutex m;
	vector_mutex() = default;
	void push_back(size_t t) {
		q.push_back(t);
	}
	size_t pop() {
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
	size_t& operator[](size_t t) {
		return q[t];
	}
};

}