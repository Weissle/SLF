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

	size_t minDepth;
	//	vector<NodeIDType> target_sequence;
	vector<pair<const NodeIDType*, const NodeIDType*>> cand_id;

	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
	shared_ptr<ShareTasks<NodeIDType>> tasks;

	inline void ToDoAfterFindASolution() {
		if (this->needOneSolution)task_distributor->end = true;
		answerReceiver << state.getMap();
	}


	bool run_(const size_t search_depth)
	{
		if (search_depth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
		auto& begin_p = cand_id[search_depth].first, & end_p = cand_id[search_depth].second;
		const auto query_id = (*match_sequence_ptr)[search_depth];
		if (search_depth)state.calCandidatePairs(query_id, begin_p, end_p);

		while (begin_p != end_p) {
			const auto target_id = *begin_p;
			begin_p++;
			if (state.checkPair(query_id, target_id)) {
				//				target_sequence[search_depth] = target_id;
				state.pushPair(query_id, target_id);
				if (run_(search_depth + 1) && task_distributor->end) return true;
				state.popPair(query_id);
			}
		}
		return false;
	}
	/*	void prepareState() {
			size_t p = 0;
			while (target_sequence[p] != NO_MAP) {
				state.pushPair((*match_sequence_ptr)[p], target_sequence[0]);
				p++;
			}
		}*/

public:
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, shared_ptr<const vector<NodeIDType>> _msp, bool _oneSolution,
		shared_ptr<const SubgraphMatchStates<GraphType>> _sp, shared_ptr<TaskDistributor<SIUnit>> _tc) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _msp, _oneSolution), answerReceiver(_answerReceiver),
		state(_q, _t, _sp), cand_id(_q.size()), task_distributor(_tc)
	{
		//	target_sequence.resize(_q.size(), NO_MAP);
	}
	void prepare(shared_ptr<ShareTasks<NodeIDType>> _tasks, size_t _min_depth) {
		minDepth = _min_depth;
		tasks = _tasks;
		return;
	}
	void run() {
		bool ok;
		NodeIDType query_id;
		while (tasks->size()) {
			if (task_distributor->end)break;
			auto target_id = tasks->getTask(&ok);
			if (!ok)continue;
			query_id = (*match_sequence_ptr)[minDepth];
			if (state.checkPair(query_id, target_id)) {
				state.pushPair(query_id, target_id);
				run_(minDepth + 1);
				state.popPair(query_id);
			}

		}

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
		};
		LOOP(i, 0, _thread_num) {
			task_distributor->addTask(f);
		}

	}
	void run() {

		auto tasksDistribute = [&](shared_ptr<ShareTasks<NodeIDType>> tasks) {
			if (task_distributor->end)return;
			bool ok = false;
			auto freeUnit = move(task_distributor->getFreeUnit(ok));
			assert(ok);
			freeUnit->prepare(tasks, 0);
			tasks.reset();
			freeUnit->run();
			task_distributor->addFreeUnit(move(freeUnit));

		};
		NodeIDType* tasks = new NodeIDType[targetGraphPtr->size()];
		for (auto i = 0; i < targetGraphPtr->size(); ++i) {
			tasks[i] = i;
		}
		bool ok;
		shared_ptr<ShareTasks<NodeIDType>> task_container = task_distributor->getShareTasksContainer(&ok);
		if (ok == false) {
			cout << "error occur : " << __FILE__ << __LINE__ << endl;
			exit(0);
		}
		task_container->addTask(tasks, tasks + targetGraphPtr->size());
		delete[]tasks;

		for (auto i = 0; i < task_distributor->threadNum(); ++i) {
			task_distributor->addTask(tasksDistribute, task_container);
		}
		cout << task_container.use_count() << endl;
		task_container.reset();
		task_distributor->join();
		return;
	}

};
}