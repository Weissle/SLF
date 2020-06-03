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
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	const GraphType* queryGraphPtr, * targetGraphPtr;
	AnswerReceiverType& answerReceiver;
	State<GraphType> state;
	vector<vector<NodeIDType>> cand_id;

	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
	shared_ptr<ShareTasks> tasks, next_tasks;

	size_t min_depth;
	size_t renewMinDepth(){
		assert(tasks->size()==0);
		while (min_depth < queryGraphPtr->size() && cand_id[min_depth].empty())++min_depth;
		return min_depth;
	}
	inline void ToDoAfterFindASolution() {
		if (_limits && answerReceiver.solutionsCount() >= _limits) task_distributor->setEnd(true);
		answerReceiver << state.getMap();
	}

	void distributeTask() {
		bool ok;
		next_tasks = task_distributor->getShareTasksContainer(&ok);
		if (!ok)return;

	//	auto min_depth = minDepth();
		renewMinDepth();
		if (min_depth == queryGraphPtr->size()) return;

		//cand_id[min_depth] will be empty
		next_tasks->giveTask(cand_id[min_depth]);

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
			if(task_distributor->haveQuality(renewMinDepth()))	distributeTask();
		}

		while (cand_id[search_depth].size()) {
			const auto target_id = cand_id[search_depth].back();
			cand_id[search_depth].pop_back();
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
		}
	}

public:
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, shared_ptr<const vector<NodeIDType>> _msp, size_t __limits,
		shared_ptr<const SubgraphMatchStates<GraphType>> _sp, shared_ptr<TaskDistributor<SIUnit>> _tc) :queryGraphPtr(&_q), targetGraphPtr(&_t),
		SubgraphIsomorphismBase(_msp, __limits), answerReceiver(_answerReceiver),
		state(_q, _t, _sp), cand_id(_q.size()), task_distributor(_tc)
	{
	}
	void prepare(shared_ptr<ShareTasks> _tasks) {
		next_tasks= move(_tasks);
		return;
	}
	void run() {
		NodeIDType query_id;
		bool ok;
		do {
			prepare_all();
			query_id = (*match_sequence_ptr)[state.depth()];
	
			while (tasks->size()) {
				if (task_distributor->end())break;
				auto target_id = tasks->getTask();
				if (target_id == NO_MAP)continue;
				if (state.checkPair(query_id, target_id)) {
					assert(state.getMap(false)[query_id] == NO_MAP);
					state.pushPair(query_id, target_id);
					run_();
					if(task_distributor->end())return;
					state.popPair(query_id);
				}
			}
			tasks.reset();
			if (next_tasks.use_count() == 0) {
				next_tasks = task_distributor->chooseSearchTasks(&ok);
				if (ok == false) return;
			}
		} while (next_tasks.use_count());
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
			task_distributor->addShareTasksContainer(make_shared<ShareTasks>());
			task_distributor->addShareTasksContainer(make_shared<ShareTasks>());
		};
		LOOP(i, 0, _thread_num) {
			task_distributor->addThreadTask(f);
		}
	}
	void run() {
		size_t first_task_num = targetGraphPtr->size();
		//prepare the first task , all target nodes.
		vector<NodeIDType> root_task(first_task_num);
		for (auto i = 0; i < first_task_num; ++i)root_task[i] = i;
		bool ok;
		shared_ptr<ShareTasks> task_container = task_distributor->getShareTasksContainer(&ok);
		if (ok == false) {
			cout << "error occur : " << __FILE__ << __LINE__ << endl;
			exit(0);
		}
		task_container->giveTask(root_task);

		while (task_distributor->runningThreadNum() || task_distributor->restTaskNum());
		task_distributor->addSearchTasks(task_container);
		task_container.reset();
		task_distributor->join();
		return;
	}

};
}