#pragma once
#include <condition_variable>
#include <vector>
#include <mutex>
#include <utility>
#include <memory>
#include "si/AnswerReceiver.hpp"
#include "si/Tasks.hpp"
#include "si/TaskDistributor.hpp"
#include "State.hpp"
#include "graph/Graph.hpp"
#include <atomic>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "si/ShareTasksContainerPool.hpp"


using namespace std;
mutex m;
namespace wg {
template<typename EdgeLabel>
class MatchUnit {
	using GraphType = GraphS<EdgeLabel>;
	using MU = MatchUnit<EdgeLabel>;
	using ShareTasksType = ShareTasks<EdgeLabel>;

	const GraphType* queryGraphPtr;
	AnswerReceiverThread *answerReceiver;
	State<EdgeLabel> state;
	vector<Tasks<EdgeLabel>> cand_id;

	TaskDistributor<EdgeLabel> *task_distributor;
	ShareTasksContainerPool<EdgeLabel> *share_tasks_container_pool;
	shared_ptr<ShareTasksType> tasks, next_tasks;

	//shared_ptr<const vector<NodeIDType>> match_sequence_ptr;
	vector<NodeIDType> matchSequence;

	size_t _limits; //how many solutions you need , _limits == 0 means no _limits;
	size_t min_depth=0;
	size_t solutions_count = 0;//Only use if we don't print out solutions .

	size_t renewMinDepth(){
		while (min_depth < queryGraphPtr->Size() && cand_id[min_depth].empty())++min_depth;
		return min_depth;
	}

	inline void ToDoAfterFindASolution() {
		if (answerReceiver -> printSolution()) {
			(*answerReceiver) << state.GetMapping();
			if (answerReceiver -> solutionsCount() >= _limits) task_distributor->SetOver();
			return;
		}
		++solutions_count;
		if ( _limits && ((_limits - answerReceiver -> solutionsCount()) / task_distributor->threadNum()) <= solutions_count ) {
			answerReceiver -> solutionCountAdd(solutions_count);
			solutions_count = 0;
			if (answerReceiver -> solutionsCount() >= _limits) task_distributor->SetOver();
		}
	}

	void distributeTask() {
		//if (min_depth == queryGraphPtr->Size()) return;
		assert (min_depth != queryGraphPtr->Size());
		next_tasks = share_tasks_container_pool->get();

		//cand_id[min_depth] will be empty
		next_tasks->giveTasks(cand_id[min_depth]);
		// cout<<"new share task"<<endl;

		auto& seq = next_tasks->targetSequence();
		seq.reserve(min_depth);
		seq = tasks->targetSequence();
		const auto& state_seq = state.GetMapping(false);
		for (int i = seq.size(); i < min_depth; ++i){ 
			seq.push_back(state_seq[matchSequence[i]]);
		}
		task_distributor->PassSharedTasks(next_tasks);
	}
	
	void ParallelSearch()
	{
		const auto search_depth = state.depth();
		if (state.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution();
			return;
		}
		const auto query_id = matchSequence[search_depth];
		state.calCandidatePairs(query_id, cand_id[search_depth]);
		if (task_distributor->allowDistribute()) {
			// assert(next_tasks.use_count()==0 || next_tasks->empty() == true);
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
		// according to the match order
		const auto& to_seq = tasks->targetSequence();
		// the whole map
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
		min_depth = tasks->targetSequence().size() + 1;
		prepareState();
	}
	void ParallelSearchOuter() {

		//chrono::time_point<chrono::steady_clock> start__ = chrono::steady_clock::now();
		prepare_all();
		//prepare_time_ += chrono::steady_clock::now() - start__;

		const NodeIDType query_id = matchSequence[state.depth()];
		// query_id = 
		int temp = state.depth();
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

		answerReceiver->solutionCountAdd(solutions_count);
		solutions_count = 0;
	}

public:
	//chrono::duration<double> work_time_;	
	//chrono::duration<double> prepare_time_;	
	MatchUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverThread *_answerReceiver, vector<NodeIDType> _msp, size_t __limits,
			shared_ptr<const SubgraphMatchStates<EdgeLabel>> _sp, TaskDistributor<EdgeLabel> *_tc,ShareTasksContainerPool<EdgeLabel> *_stcp) :
		queryGraphPtr(&_q), matchSequence(_msp),_limits(__limits),
	answerReceiver(_answerReceiver),state(_q, _t, _sp), cand_id(_q.Size()), task_distributor(_tc),share_tasks_container_pool(_stcp){ 
		//work_time_ = work_time_.zero();
		//prepare_time_ = prepare_time_.zero();
	}

	void run(){
		shared_ptr<condition_variable> wakeupCV = task_distributor->wakeupCV;
		mutex __;
		//is not over
		while(task_distributor->end() == false){

			if(tasks == nullptr || tasks->empty()) {
				// is over or has task.
				task_distributor->ReportBecomeLeisure();
				unique_lock<mutex> ul(__);
				wakeupCV->wait(ul,[this](){	return this->task_distributor->end() || this->task_distributor->taskAvaliable();});
				tasks = task_distributor->ChooseHeavySharedTask();
				task_distributor->ReportBecomeBusy();
				if(tasks == nullptr) continue;
			}

			//chrono::time_point<chrono::steady_clock> start__ = chrono::steady_clock::now();
			ParallelSearchOuter();

			if(next_tasks !=nullptr && next_tasks->empty()==false) tasks = move(next_tasks);
			else tasks = task_distributor->ChooseHeavySharedTask();
			//work_time_ += chrono::steady_clock::now() - start__;
			
		}
		m.lock();
		//cout<<"thread id : "<<this_thread::get_id()<<' '<< work_time_.count()<<' '<<prepare_time_.count()<<endl;
		m.unlock();
	}

};

template<typename EdgeLabel>
class ParallelSubgraphIsomorphism{
	using MU = MatchUnit<EdgeLabel> ;
	using GraphType = GraphS<EdgeLabel>;
public:
	ParallelSubgraphIsomorphism() = default;
	void run(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverThread *_answer_receiver, size_t _thread_num,size_t __limits,const vector<NodeIDType>& _match_sequence){

		TaskDistributor<EdgeLabel> *task_distributor = new TaskDistributor<EdgeLabel>(_thread_num);
		ShareTasksContainerPool<EdgeLabel> *share_tasks_container_pool_ptr = new ShareTasksContainerPool<EdgeLabel>(_thread_num);
		size_t first_task_num = _targetGraph.Size();
		//auto rootTask = task_distributor->getShareTasksContainer();
		auto rootTask = share_tasks_container_pool_ptr->get();
		rootTask->giveTasks(first_task_num);
		task_distributor->PassSharedTasks(rootTask);

		auto subgraph_states = makeSubgraphState<EdgeLabel>(_queryGraph, _match_sequence);
		auto f = [&]() {
			auto p = MU(_queryGraph, _targetGraph, _answer_receiver, _match_sequence, __limits, subgraph_states, task_distributor,share_tasks_container_pool_ptr);
			task_distributor->ReportBecomeBusy();
			p.run();
		};
		vector<thread> match_units(_thread_num);
		for (int i = 0; i < _thread_num; ++i){ 
			match_units[i] = thread(f);
		}
		
		for (int i = 0; i < _thread_num; ++i){ 
			if(match_units[i].joinable()) match_units[i].join();
			//assert(task_distributor->end());
		}
		
		delete task_distributor;
		delete share_tasks_container_pool_ptr;
	}
};
}
