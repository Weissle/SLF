#pragma once
#include<vector>
#include"SubgraphIsomorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"ThreadRelatedClass.hpp"
#include<mutex>
#include<utility>
#include<memory>
#include"tools/ThreadPool.hpp"
#include"si/TaskDistributor.hpp"
#include<atomic>
#include<time.h>
#include<assert.h>
#include<iostream>
using namespace std;
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase{
public:
	using EdgeType = typename GraphType::EdgeType;
private:
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	using ShareTasksType = ShareTasks<EdgeType>;
	const GraphType* queryGraphPtr, * targetGraphPtr;
	AnswerReceiverType& answerReceiver;
	State<GraphType> state;
	vector<Tasks<EdgeType>> cand_id;

	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
	shared_ptr<ShareTasksType> tasks, next_tasks;

	size_t min_depth;
	size_t solutions_count = 0;//Only use if we don't print out solutions and no limit of solutions number;
	size_t renewMinDepth(){
		assert(tasks->size()==0);
		while (min_depth < queryGraphPtr->size() && cand_id[min_depth].empty())++min_depth;
		return min_depth;
	}

	inline void ToDoAfterFindASolution() {
		if (_limits || answerReceiver.printSolution()) {
			answerReceiver << state.getMap();
			if (answerReceiver.solutionsCount() >= _limits) task_distributor->setEnd(true);
		}
		else ++solutions_count;
	}

	void distributeTask() {
		bool ok;
		next_tasks = task_distributor->getShareTasksContainer();

	//	auto min_depth = minDepth();
		renewMinDepth();
		if (min_depth == queryGraphPtr->size()) return;

		//cand_id[min_depth] will be empty
		next_tasks->giveTasks(cand_id[min_depth]);

		auto& seq = next_tasks->targetSequence();
		seq = tasks->targetSequence(); //.assign(tasks->targetSequence().begin(), tasks->targetSequence().end());
		const auto& state_seq = state.getMap(false);
		for (auto i = seq.size(); i < min_depth; ++i) {
			seq.push_back(state_seq[(*match_sequence_ptr)[i]]);
		}
		
		task_distributor->addThreadTask(&TaskDistributor<SIUnit>::addSearchTasks, task_distributor.get(),next_tasks);
	}
	
	void run_()
	{
		const auto search_depth = state.depth();
		if (search_depth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return;
		}
		const auto query_id = (*match_sequence_ptr)[search_depth];
		state.calCandidatePairs(query_id, cand_id[search_depth]);
		if (task_distributor->allowDistribute() && tasks->size() == 0 && (next_tasks.use_count() == 0 || next_tasks->size() == 0)) {
			next_tasks.reset();
			if(task_distributor->haveQuality(renewMinDepth()))	distributeTask();
		}

		while (cand_id[search_depth].size()) {
			const auto target_id = cand_id[search_depth].getTask();
			if (state.checkPair(query_id, target_id)) {
				state.pushPair(query_id, target_id);
				run_();
				if (task_distributor->end()) return;
				state.popPair(query_id);
			}
		}

		
	}

	void prepareState() {
		const auto& to_seq = tasks->targetSequence();
		const auto& state_seq = state.getMap(false);
		size_t diff_point = 0;
		for (diff_point = 0; diff_point < to_seq.size(); ++diff_point) {
			if (state_seq[(*match_sequence_ptr)[diff_point]] != to_seq[diff_point])break;
		}

		while (state.depth() > diff_point) { 
			const auto pop_query_node = (*match_sequence_ptr)[state.depth() - 1];
			state.popPair(pop_query_node); 
		}
		for (auto i = diff_point; i < to_seq.size(); ++i) {
			state.pushPair((*match_sequence_ptr)[i], to_seq[i]);
		}
	}
	void prepare_all() {
		tasks=move(next_tasks);
		if (tasks->size()) {
			min_depth = tasks->targetSequence().size() + 1;
			prepareState();
	/*		if (tasks->empty()) {
				cout << "prepare all but no task" << endl;
			}*/
		}
	}

public:
	
	clock_t run_clock = 0;
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, shared_ptr<const vector<NodeIDType>> _msp, size_t __limits,
		shared_ptr<const SubgraphMatchStates<GraphType>> _sp, shared_ptr<TaskDistributor<SIUnit>> _tc) :queryGraphPtr(&_q), targetGraphPtr(&_t),
		SubgraphIsomorphismBase(_msp, __limits), answerReceiver(_answerReceiver),
		state(_q, _t, _sp), cand_id(_q.size()), task_distributor(_tc)
	{
	}
	void prepare(shared_ptr<ShareTasksType> _tasks) {
		next_tasks= move(_tasks);
		return;
	}
	void run() {
		NodeIDType query_id;
		bool ok;
		do {
	//		auto c1 = clock();
			prepare_all();
			query_id = (*match_sequence_ptr)[state.depth()];
	
			while (tasks->size()) {
				auto target_id = tasks->getTask();
				if (target_id == NO_MAP)continue;
				if (state.checkPair(query_id, target_id)) {
					state.pushPair(query_id, target_id);
					run_();
					if(task_distributor->end())return;
					state.popPair(query_id);
				}
			}
			tasks.reset();
	//		run_clock += clock() - c1;
			if (next_tasks.use_count() == 0) {
				next_tasks = task_distributor->chooseSearchTasks(&ok);
				if (ok == false) break;
			}
		} while (next_tasks.use_count());
		answerReceiver.solutionCountAdd(solutions_count);
		solutions_count = 0;
	}

};

template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase {
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	const GraphType* queryGraphPtr, * targetGraphPtr;
	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answer_receiver, size_t _thread_num,
		size_t __limits, shared_ptr<const vector<NodeIDType>>& _match_sequence_ptr):queryGraphPtr(&_queryGraph), targetGraphPtr(&_targetGraph),
		SubgraphIsomorphismBase(_match_sequence_ptr, __limits), task_distributor(make_shared<TaskDistributor<SIUnit>>(_thread_num))
	{
		auto subgraph_states = makeSubgraphState<GraphType>(_queryGraph, _match_sequence_ptr);
		auto f = [&, subgraph_states,__limits]() {
			auto p = make_unique<SIUnit>(*queryGraphPtr, *targetGraphPtr, _answer_receiver, _match_sequence_ptr, __limits, subgraph_states, task_distributor);
			task_distributor->addFreeUnit(move(p));
		};
		LOOP(i, 0, _thread_num) {
			task_distributor->addThreadTask(f);
		}
	}
	void run() {
		size_t first_task_num = targetGraphPtr->size();

		auto task_container = task_distributor->getShareTasksContainer();
		task_container->giveTasks(first_task_num);

		while (task_distributor->runningThreadNum() || task_distributor->restTaskNum());
		task_distributor->addSearchTasks(task_container);
		task_container.reset();
		task_distributor->join();
//		task_distributor->output_hittimes();
		return;
	}

};
}