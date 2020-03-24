#pragma once;
#include<utility>
#include<stack>
#include<mutex>
#include<assert.h>
#include<limits.h>
using namespace std;
namespace wg {
template<class _Ty>
class stack_mutex {
	stack<_Ty> s;
	std::mutex m;
public:
	stack_mutex() = default;
	void push(_Ty t) {
		lock_guard<mutex> lg(m);
		s.push(move(t));
	}
	_Ty pop(bool& ok) {
		lock_guard<mutex> lg(m);
		if (s.empty()) {
			ok = false;
			return move(_Ty());
		}
		else {
			auto t = move(s.top());
			s.pop();
			ok = true;
			return move(t);
		}
	}
	bool empty() const {
		return s.empty();
	}
	size_t size() const {
		return s.size();
	}

};

template<class T>
class DynamicArray {
	T* p = nullptr;
	size_t size_ = 0;
public:
	DynamicArray() = default;
	~DynamicArray() {
		size_ = 0;
		if (p)delete[]p;
		p = nullptr;
	}
	DynamicArray(size_t max) :size_(max) {
		p = new T[max]();
	}
	inline T& operator[](const size_t place) {
		return p[place];
	}
	inline const T& operator[](const size_t place)const {
		return p[place];
	}
	DynamicArray(const DynamicArray<T>& other) {
		size_ = other.size_;
		p = new T[size_];
		std::copy(other.p, other.p + size_, p);
	}
	DynamicArray<T>& operator=(const DynamicArray<T>& other) {
		if (this == &other)return *this;
		if (size_ != other.size_) {
			if (p)delete[]p;
			p = new T[size_];
		}
		size_ = other.size_;
		std::copy(other.p, other.p + size_, p);
		return *this;
	}
	inline T* data() {
		return p;
	}
	inline size_t size()const { return size_; }
	inline T* begin() { return p; }
	inline T* end() { return p + size_; }
	inline const T* begin()const { return p; }
	inline const T* end()const { return p + size_; }
};

class bitmap {
	vector<size_t> p1;
	DynamicArray<size_t> p;
	static size_t* for_true;
	static size_t* for_false;
	static bool usable;
public:
	bitmap() = default;
	bitmap(size_t max) {
		int size = max / 64 + ((max % 64 == 0) ? 0 : 1);
		p = DynamicArray<size_t>(size);
		if (usable == false) {
			usable = true;
			for_true = new size_t[64];
			for_false = new size_t[64];
			size_t temp = 1;
			LOOP(i, 0, 64) {
				for_true[i] = temp;
				temp <<= 1;
				for_false[i] = for_true[i] ^ SIZE_MAX;
			}
		}
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
