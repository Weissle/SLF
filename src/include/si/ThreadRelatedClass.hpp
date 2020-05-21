#pragma once;
#include<utility>
#include<stack>
#include<mutex>
#include<assert.h>
#include<limits.h>
#include"si/si_marcos.h"
using namespace std;
namespace wg {
class ShareTasks {
	mutex m;
	vector<NodeIDType> _task;
	vector<NodeIDType> target_sequence;
public:
	ShareTasks() = default;
	size_t size()const { return _task.size(); }
	
	template<class _It>
	void addTask(const _It begin, const _It end) {
		lock_guard<mutex> lg(m);
		_task.assign(begin, end);
	}
	NodeIDType getTask() {
		lock_guard<mutex> lg(m);
		if(_task.empty())return NO_MAP;
		else {
			const auto answer = _task.back();
			_task.pop_back();
			return answer;
		}
	}
	const vector<NodeIDType>& getTargetSequence()const { return target_sequence; }
	template<class _It>
	void setTargetSequence(const _It first, const _It end) { 
		lock_guard<mutex> lg(m);
		target_sequence.assign(first, end); 
	}
	vector<NodeIDType>& targetSeq() { return target_sequence; }
	bool empty()const { return _task.empty(); }
};

template<class _T>
class TwoDArray {
	_T* _p = nullptr;
	size_t _row, _col;
public:
	TwoDArray() = default;
	TwoDArray(size_t row_, size_t col_):_row(row_),_col(col_) {
		_p = new _T[row_ * col_]();
	}
	TwoDArray(const TwoDArray<_T>& r):_row(r._row),_col(r._col) {
		_p = new _T[_row * _col]();
		copy(r._p, r._p + (_row * _col), _p);
	}
	TwoDArray<_T>& operator=(const TwoDArray<_T>& r) {
		if (&r == this)return *this;
		else {
			delete[]_p;
			_row = r._row;
			_col = r._col;
			_p = new _T[_row * _col]();
			copy(r._p, r._p + (_row * _col), _p);
		}
		return *this;
	}
	size_t row()const { return _row; }
	size_t column()const { return _col; }
	_T* operator[](size_t row)const {
		assert(row < _row);
		return _p + (row * _col);
	}
	_T* get()const {
		return _p;
	}
	~TwoDArray() {
		delete[]_p;
	}
};
class bitmap {
	vector<size_t> p;
	static size_t* for_true;
	static size_t* for_false;
	static bool usable;
	const static size_t bits = sizeof(size_t) * 8;
public:
	bitmap() = default;
	bitmap(size_t max) {
		int size = max / bits + ((max % bits == 0) ? 0 : 1);
		p = vector<size_t>(size);
		if (for_true==nullptr && for_false==nullptr) {
			for_true = new size_t[bits];
			for_false = new size_t[bits];
			size_t temp = 1;
			LOOP(i, 0, bits) {
				for_true[i] = temp;
				temp <<= 1;
				for_false[i] = for_true[i] ^ SIZE_MAX;
			}
			usable = true;
		}
		while (usable == false);
	}
	inline bool content(size_t place) const {
		return (p[place / bits] & for_true[place % bits]);
	}
	inline void setTrue(size_t place) {
		p[place / bits] |= for_true[place % bits];
	}
	inline void setFalse(size_t place) {
		p[place / bits] &= for_false[place % bits];
	}

};
bool bitmap::usable = false;
size_t* bitmap::for_false = nullptr;
size_t* bitmap::for_true = nullptr;

}
