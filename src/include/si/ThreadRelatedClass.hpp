#pragma once;
#include<utility>
#include<stack>
#include<mutex>
#include<assert.h>
using namespace std;
namespace wg {
template<class _Ty>
class stack_mutex {
	stack<_Ty> s;
	std::mutex m;
public:
	stack_mutex() = default;
	void push(const _Ty& t) {
		lock_guard<mutex> lg(m);
		s.push(t);
	}
	_Ty pop(bool& ok) {
		lock_guard<mutex> lg(m);
		if (s.empty()) {
			ok = false;
			return _Ty();
		}
		else {
			auto t = s.top();
			s.pop();
			ok = true;
			return t;
		}
	}
	bool empty() const {
		return s.empty();
	}
	size_t size() const {
		return s.size();
	}

};

}