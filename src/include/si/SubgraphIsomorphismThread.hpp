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
//mutex m;
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
		while (state.depth() > diff_point) { state.popPair((*match_sequence_ptr)[state.depth() - 1]); }
		for (auto i = diff_point; i < to_seq.size(); ++i) {
			state.pushPair((*match_sequence_ptr)[diff_point], to_seq[i]);
		}
	}
	void prepare_all() {
		if (next_tasks->size()) {
			tasks.swap(next_tasks);
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
		next_tasks.swap(_tasks);
		//	tasks = _tasks;

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
					state.pushPair(query_id, target_id);
					run_();
					state.popPair(query_id);
				}
				
			}
		} while (next_tasks.use_count());

		tasks.reset();
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
		auto fut = task_distributor->addTask([]() {return true; });
		fut.get();
	}
	void run() {

		NodeIDType* tasks = new NodeIDType[targetGraphPtr->size()];
		for (auto i = 0; i < targetGraphPtr->size(); ++i) {
			tasks[i] = i;
		}
		bool ok;
		shared_ptr<ShareTasks> task_container = task_distributor->getShareTasksContainer(&ok);
		if (ok == false) {
			cout << "error occur : " << __FILE__ << __LINE__ << endl;
			exit(0);
		}
		task_container->addTask(tasks, tasks + targetGraphPtr->size());
		delete[]tasks;

		cout << task_container.use_count() << endl;
		task_distributor->addTasks(move(task_container));
		cout << task_container.use_count() << endl;
		
		task_distributor->join();
		return;
	}

};
}