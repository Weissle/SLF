#pragma once
#include <condition_variable>
#include <vector>
#include "SubgraphIsomorphism.hpp"
#include "State.hpp"
#include "graph/Graph.hpp"
#include <mutex>
#include <utility>
#include <memory>
#include "si/AnswerReceiver.hpp"
#include "tools/ThreadPool.hpp"
#include "si/TaskDistributor.hpp"
#include <atomic>
#include <time.h>
#include <assert.h>
#include <iostream>
using namespace std;
namespace wg {
template<typename EdgeLabel>
class MatchUnit {
	using GraphType = GraphS<EdgeLabel>;
	using MU = MatchUnit<EdgeLabel>;
	using ShareTasksType = ShareTasks<EdgeLabel>;

	const GraphType* queryGraphPtr, * targetGraphPtr;
	AnswerReceiverThread *answerReceiver;
	State<EdgeLabel> state;
	vector<Tasks<EdgeLabel>> cand_id;

	TaskDistributor<EdgeLabel> *task_distributor;
	shared_ptr<ShareTasksType> tasks, next_tasks;

	//shared_ptr<const vector<NodeIDType>> match_sequence_ptr;
	vector<NodeIDType> matchSequence;

	size_t _limits; //how many _limits you need , _limits == 0 means no _limits;
	size_t min_depth=0;
	size_t solutions_count = 0;//Only use if we don't print out solutions .

	size_t renewMinDepth(){
		assert(tasks->size()==0);
		while (min_depth < queryGraphPtr->Size() && cand_id[min_depth].empty())++min_depth;
		return min_depth;
	}

	inline void ToDoAfterFindASolution() {
		if (answerReceiver -> printSolution()) {
			(*answerReceiver) << state.GetMapping();
			if (answerReceiver -> solutionsCount() >= _limits) task_distributor->setEnd(true);
			return;
		}
		++solutions_count;
		if ( _limits && ((_limits - answerReceiver -> solutionsCount()) / task_distributor->threadNum()) <= solutions_count ) {
			answerReceiver -> solutionCountAdd(solutions_count);
			solutions_count = 0;
			if (answerReceiver -> solutionsCount() >= _limits) task_distributor->setEnd(true);
		}
	}

	void distributeTask() {
		if (min_depth == queryGraphPtr->Size()) return;

		next_tasks = task_distributor->getShareTasksContainer();

		//cand_id[min_depth] will be empty
		next_tasks->giveTasks(cand_id[min_depth]);

		auto& seq = next_tasks->targetSequence();
		seq = tasks->targetSequence(); //.assign(tasks->targetSequence().begin(), tasks->targetSequence().end());
		const auto& state_seq = state.GetMapping(false);
		for (auto i = seq.size(); i < min_depth; ++i) {
			seq.push_back(state_seq[matchSequence[i]]);
		}
		
		task_distributor->addThreadTask(&TaskDistributor<EdgeLabel>::PassSharedTasks, task_distributor.get(),next_tasks);
		// task_distributor->PassSharedTasks(next_tasks);
	}
	
	void ParallelSearch()
	{
		const auto search_depth = state.depth();
		if (search_depth == queryGraphPtr->Size()) {
			this->ToDoAfterFindASolution();
			return;
		}
		const auto query_id = matchSequence[search_depth];
		state.calCandidatePairs(query_id, cand_id[search_depth]);
		// if (task_distributor->allowDistribute()) {
		if (task_distributor->allowDistribute() && tasks->size() == 0 && (next_tasks.use_count() == 0 || next_tasks->size() == 0)) {
			next_tasks.reset();
			if(task_distributor->haveQuality(renewMinDepth()))	distributeTask();
		}

		while (cand_id[search_depth].size()) {
			const auto target_id = cand_id[search_depth].getTask();
			if (state.AddAble(query_id, target_id)) {
				state.AddPair(query_id, target_id);
				ParallelSearch();
				if (task_distributor->end()) return;
				state.RemovePair(query_id);
			}
		}


	}

	void prepareState() {
		const auto& to_seq = tasks->targetSequence();
		const auto& state_seq = state.GetMapping(false);
		size_t diff_point = 0;
		for (diff_point = 0; diff_point < to_seq.size(); ++diff_point) {
			if (state_seq[matchSequence[diff_point]] != to_seq[diff_point])break;
		}

		while (state.depth() > diff_point) { 
			const auto pop_query_node = matchSequence[state.depth() - 1];
			state.RemovePair(pop_query_node); 
		}
		for (auto i = diff_point; i < to_seq.size(); ++i) {
			state.AddPair(matchSequence[i], to_seq[i]);
		}
	}
	void prepare_all() {
		tasks=move(next_tasks);
		if (tasks->size()) {
			min_depth = tasks->targetSequence().size() + 1;
			prepareState();
		}
	}

public:
	
	MatchUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverThread *_answerReceiver, vector<NodeIDType> _msp, size_t __limits,
		shared_ptr<const SubgraphMatchStates<EdgeLabel>> _sp, TaskDistributor<EdgeLabel> *_tc) :queryGraphPtr(&_q), targetGraphPtr(&_t),matchSequence(_msp),_limits(__limits),
		answerReceiver(_answerReceiver),state(_q, _t, _sp), cand_id(_q.Size()), task_distributor(_tc){}
	void prepare(shared_ptr<ShareTasksType> _tasks) {
		next_tasks= move(_tasks);
		return;
	}
	void ParallelSearchOuter() {
		NodeIDType query_id;
		bool ok;
		do {
			prepare_all();
			query_id = matchSequence[state.depth()];
	
			while (tasks->size()) {
				auto target_id = tasks->getTask();
				if (target_id == NO_MAP)continue;
				if (state.AddAble(query_id, target_id)) {
					state.AddPair(query_id, target_id);
					ParallelSearch();
					if(task_distributor->end())return;
					state.RemovePair(query_id);
				}
			}
			tasks.reset();
			// if (next_tasks.use_count() == 0 || next_tasks->empty()) {
			if (next_tasks.use_count() == 0) {
				next_tasks = task_distributor->ChooseHeavySharedTask(&ok);
				if (ok == false) break;
			}
		} while (next_tasks.use_count());
		answerReceiver->solutionCountAdd(solutions_count);
		solutions_count = 0;
	}
	void run(){
		shared_ptr<condition_variable> wakeupCV = task_distributor->wakeupCV;
		unique_lock<mutex> ul;
		while(task_distributor->end() == false){
			wakeupCV->wait(ul,[this](){
				return this->task_distributor->end() || this->task_distributor->taskAvaliable();
			});
			if(task_distributor->end())break;
			bool ok;
			tasks = task_distributor->ChooseHeavySharedTask(&ok);
			if(ok == false) continue;
			

			ParallelSearchOuter();
		}
	}

};

template<typename EdgeLabel>
class ParallelSubgraphIsomorphism{
	using MU = MatchUnit<EdgeLabel> ;
	using GraphType = GraphS<EdgeLabel>;
public:
	ParallelSubgraphIsomorphism() = default;
	void run(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverThread *_answer_receiver, size_t _thread_num,size_t __limits,const vector<NodeIDType>& _match_sequence){

		TaskDistributor<EdgeLabel> *task_distributor = new TaskDistributor<EdgeLabel>();
		size_t first_task_num = _targetGraph.Size();
		auto rootTask = task_distributor->getShareTasksContainer();
		rootTask->giveTasks(first_task_num);
		task_distributor->PassSharedTasks(rootTask);

		auto subgraph_states = makeSubgraphState<EdgeLabel>(_queryGraph, _match_sequence);
		auto f = [&, subgraph_states,__limits]() {
			auto p = MU(_queryGraph, _targetGraph, _answer_receiver, _match_sequence, __limits, subgraph_states, task_distributor);
			p->run();
		};
		thread match_units[_thread_num];
		for (int i = 0; i < _thread_num; ++i){ 
			match_units[i] = thread(f);
		}

		for (int i = 0; i < _thread_num; ++i){ 
			if(match_units[i].joinable()) match_units[i].join();
		}
		
		delete task_distributor;
	}
};
}
