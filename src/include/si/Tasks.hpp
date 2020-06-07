#pragma once
#include<graph/Edge.hpp>
#include<si/si_marcos.h>
#include<mutex>

using namespace std;
namespace wg{
enum class TASK_TYPE{ S,T,N };//source edge , target edge , N is from 0 to targetGraph.size()
template<class EdgeType>
class Tasks {
	TASK_TYPE type;
	const EdgeType* ptr = nullptr;
	size_t start=0, end=0;
	inline void initialTask(size_t _s, size_t _e, const EdgeType* _p, const TASK_TYPE _t) {
		start = _s;
		end = _e;
		ptr = _p;
		type = _t;
	}
public:
	Tasks() = default;
	void giveTasks(size_t _end) {
		initialTask(0, _end, nullptr, TASK_TYPE::N);
	}
	void giveTasks(size_t _end, const EdgeType* _ptr,TASK_TYPE _t) {
		initialTask(0, _end, _ptr, _t);
	}
	void giveTasks(Tasks<EdgeType>& t) {
		type = t.type;
		ptr = t.ptr;
		start = t.start;
		end = t.end;
		t.end = t.start;
	}
	const NodeIDType getTask() {
		NodeIDType answer = NO_MAP;
		if (!empty()) {
			if (type == TASK_TYPE::N) answer = start;
			else if (type == TASK_TYPE::S) answer = (ptr + start)->source();
			else if (type == TASK_TYPE::T) answer = (ptr + start)->target();
			++start;
		}
		return answer;
	}
	const bool empty() const {
		return start == end;
	}
	const bool size() const {
		return end - start;
	}
};

template<class EdgeType>
class ShareTasks :public Tasks<EdgeType>{
	mutex m;
	vector<NodeIDType> target_sequence;
public:
	ShareTasks() = default;
	const NodeIDType getTask() {
		lock_guard<mutex> lg(m);
		return Tasks<EdgeType>::getTask();
	}
	const vector<NodeIDType>& targetSequence()const { return target_sequence; }
	vector<NodeIDType>& targetSequence() { return target_sequence; }
	size_t depth()const {
		return target_sequence.size();
	}

};
}