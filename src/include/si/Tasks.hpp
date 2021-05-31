#pragma once
#include<graph/Edge.hpp>
#include<si/si_marcos.h>
#include<mutex>


using namespace std;
namespace wg{

template<typename EdgeLabel>
class State;

template<typename EdgeLabel>
class StateL;

enum class TASK_TYPE{ S,T,N };//source edge , target edge , N is from 0 to targetGraph.size()
template<class EdgeLabelType>
class Tasks {
	TASK_TYPE type;
	const void* ptr = nullptr;
	size_t start=0, end=0;
	inline void initialTask(size_t _s, size_t _e, const void* _p, const TASK_TYPE _t) {
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
	void giveTasks(size_t _end, const void* _ptr,TASK_TYPE _t) {
		initialTask(0, _end, _ptr, _t);
	}
	void giveTasks(Tasks<EdgeLabelType>& t) {
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
			else if (type == TASK_TYPE::S) {
				auto ep = static_cast<const SourceEdge<EdgeLabelType>*>(ptr);
			//	answer = (ptr + start)->source();
				answer = (ep + start)->source();
			}
			else if (type == TASK_TYPE::T) {
				auto ep = static_cast<const TargetEdge<EdgeLabelType>*>(ptr);
				answer = (ep + start)->target();
			}
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

template<class EdgeLabelType>
class ShareTasks :public Tasks<EdgeLabelType>{
	mutex m;
	vector<NodeIDType> target_sequence;
public:
	State<EdgeLabelType> *state;
	ShareTasks(){
		state = new State<EdgeLabelType>();
	}
	const NodeIDType getTask() {
		lock_guard<mutex> lg(m);
		return Tasks<EdgeLabelType>::getTask();
	}
	const vector<NodeIDType>& targetSequence()const { return target_sequence; }
	vector<NodeIDType>& targetSequence() { return target_sequence; }
	size_t depth()const {
		return target_sequence.size();
	}
	~ShareTasks(){
		delete state;
	}
};
}
