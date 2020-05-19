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
mutex m;
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	AnswerReceiverType& answerReceiver;
	State<GraphType> state;

	vector<pair<const NodeIDType*, const NodeIDType*>> cand_id;

	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
	shared_ptr<ShareTasks> tasks, next_tasks;

	inline void ToDoAfterFindASolution() {
		if (this->needOneSolution)task_distributor->end = true;
		answerReceiver << state.getMap();
	}
	void distributeTask() {
		bool ok;
		next_tasks = task_distributor->getShareTasksContainer(&ok);
		if (!ok)return;

		size_t min_depth = tasks->getTargetSequence().size() + 1;
		while (min_depth < queryGraphPtr->size() && cand_id[min_depth].first == cand_id[min_depth].second)++min_depth;
		if (min_depth == queryGraphPtr->size()) return;

		next_tasks->addTask(cand_id[min_depth].first, cand_id[min_depth].second);
		cand_id[min_depth].second = cand_id[min_depth].first = nullptr;
		auto& seq = next_tasks->targetSeq();
		const auto& state_seq = state.getMap(false);
		seq.clear();
		for (auto i = 0; i < min_depth; ++i) {
			seq.push_back(state_seq[(*match_sequence_ptr)[i]]);
		}
		task_distributor->addTasks(next_tasks);
	}

	void run_()
	{
		const auto search_depth = state.depth();
		if (search_depth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return;
		}
		auto& begin_p = cand_id[search_depth].first, & end_p = cand_id[search_depth].second;
		const auto query_id = (*match_sequence_ptr)[search_depth];
		state.calCandidatePairs(query_id, begin_p, end_p);

		if (task_distributor->allowDistribute() && tasks->size() == 0 && next_tasks.use_count() == 0) {
			distributeTask();
		}

		while (begin_p != end_p) {
			const auto target_id = *begin_p;
			begin_p++;
			if (state.checkPair(query_id, target_id)) {
				state.pushPair(query_id, target_id);
				run_();
				if (task_distributor->end) return;
				state.popPair(query_id);
			}
		}

		
	}

	void prepareState() {
		const auto& to_seq = tasks->getTargetSequence();
		const auto& state_seq = state.getMap(false);
		size_t diff_point = 0;
		for (diff_point = 0; diff_point < to_seq.size(); ++diff_point) {
			if (state_seq[(*match_sequence_ptr)[diff_point]] != to_seq[diff_point])break;
		}
		if (state.depth() != 0) {
			int a = 0;
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
			prepareState();
		}
	}

public:
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, shared_ptr<const vector<NodeIDType>> _msp, bool _oneSolution,
		shared_ptr<const SubgraphMatchStates<GraphType>> _sp, shared_ptr<TaskDistributor<SIUnit>> _tc) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _msp, _oneSolution), answerReceiver(_answerReceiver),
		state(_q, _t, _sp), cand_id(_q.size()), task_distributor(_tc)
	{
		//	target_sequence.resize(_q.size(), NO_MAP);
	}
	void prepare(shared_ptr<ShareTasks> _tasks) {
		next_tasks= move(_tasks);
		return;
	}
	void run() {
		NodeIDType query_id;
		do {
			prepare_all();
			query_id = (*match_sequence_ptr)[state.depth()];
	
			while (tasks->size()) {
				if (task_distributor->end)break;
				auto target_id = tasks->getTask();
				if (target_id == NO_MAP)continue;
				if (state.checkPair(query_id, target_id)) {
					assert(state.getMap(false)[query_id] == NO_MAP);
					state.pushPair(query_id, target_id);
					run_();
					state.popPair(query_id);
				}
				
			}
			tasks.reset();
		} while (next_tasks.use_count());
	}

};

template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase<GraphType> {
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answer_receiver, size_t _thread_num,
		bool _onlyNeedOneSolution, shared_ptr<const vector<NodeIDType>>& _match_sequence_ptr)
		:SubgraphIsomorphismBase<GraphType>(_queryGraph, _targetGraph, _match_sequence_ptr, _onlyNeedOneSolution), task_distributor(make_shared<TaskDistributor<SIUnit>>(_thread_num))
	{
		auto subgraph_states = makeSubgraphState<GraphType>(_queryGraph, _match_sequence_ptr);
		auto f = [&, subgraph_states]() {
			auto p = make_unique<SIUnit>(*queryGraphPtr, *targetGraphPtr, _answer_receiver, _match_sequence_ptr, _onlyNeedOneSolution, subgraph_states, task_distributor);
			task_distributor->addFreeUnit(move(p));
			task_distributor->addShareTasksContainer(make_shared<ShareTasks>());
			task_distributor->addShareTasksContainer(make_shared<ShareTasks>());
		};
		LOOP(i, 0, _thread_num) {
			task_distributor->addTask(f);
		}
	}
	void run() {
		//size_t first_task_num = min(size_t(1),targetGraphPtr->size());
		size_t first_task_num = targetGraphPtr->size();
		//prepare the first task , all target nodes.
		NodeIDType* tasks = new NodeIDType[first_task_num];
		for (auto i = 0; i < first_task_num; ++i) 	tasks[i] = i;
		bool ok;
		shared_ptr<ShareTasks> task_container = task_distributor->getShareTasksContainer(&ok);
		if (ok == false) {
			cout << "error occur : " << __FILE__ << __LINE__ << endl;
			exit(0);
		}
		task_container->addTask(tasks, tasks + first_task_num);
		delete[]tasks;

		task_distributor->addTasks(move(task_container));
		
		task_distributor->join();
		return;
	}

};
}