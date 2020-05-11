#pragma once;
#include<utility>
#include<stack>
#include<mutex>
#include<assert.h>
#include<limits.h>
using namespace std;
namespace wg {
template<class _T>
class ShareTasks {
	mutex m;
	vector<_T> _v;
public:
	ShareTasks() = default;
	size_t size()const { return _v.size(); }
/*	void addTask(_T t) {
		lock_guard<mutex> lg(m);
		_v.push_back(move(t));
	}*/
	template<class _It>
	void addTask(const _It begin, const _It end) {
		lock_guard<mutex> lg(m);
		_v.assign(begin, end);
	}
	_T getTask(bool* ok) {
		lock_guard<mutex> lg(m);
		if (_v.size()) {
			*ok = true;
			auto temp = move(_v.back());
			_v.pop_back();
			return move(temp);
		}
		else {
			*ok = false;
			return _T();
		}
	}

};


class bitmap {
	vector<size_t> p;
	static size_t* for_true;
	static size_t* for_false;
	static bool usable;
public:
	bitmap() = default;
	bitmap(size_t max) {
		int size = max / 64 + ((max % 64 == 0) ? 0 : 1);
		p = vector<size_t>(size);
		if (for_true==nullptr && for_false==nullptr) {
			for_true = new size_t[64];
			for_false = new size_t[64];
			size_t temp = 1;
			LOOP(i, 0, 64) {
				for_true[i] = temp;
				temp <<= 1;
				for_false[i] = for_true[i] ^ SIZE_MAX;
			}
			usable = true;
		}
		while (usable == false);
	}
	inline bool content(size_t place) const {
		return (p[place / 64] & for_true[place % 64]);
	}
	inline void setTrue(size_t place) {
		p[place / 64] |= for_true[place % 64];
	}
	inline void setFalse(size_t place) {
		p[place / 64] &= for_false[place % 64];
	}

};
bool bitmap::usable = false;
size_t* bitmap::for_false = nullptr;
size_t* bitmap::for_true = nullptr;

}
