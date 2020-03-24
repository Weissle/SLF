#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIsomorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"MatchOrderSelector.hpp"
#include"ThreadRelatedClass.hpp"
#include<mutex>
#include<utility>
#include<memory>
#include"tools/ThreadPool.hpp""
#include<atomic>
#include<time.h>
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	AnswerReceiverType& answerReceiver;
	size_t searchDepth = 0, maxDepth = 0;
	State<GraphType> state;
	atomic_bool& end;

	size_t minDepth;
	State<GraphType> min_state;
	vector<NodeIDType> target_sequence;
	DynamicArray<pair<const NodeIDType*, const NodeIDType*>> cand_id;
	shared_ptr<ThreadPool> thread_pool;
	shared_ptr<stack_mutex<unique_ptr<SIUnit>>> free_units, prepare_units;
	inline void ToDoAfterFindASolution() {
		answerReceiver << state.getMap();
	}
	inline void popOperation() {
		searchDepth--;
		state.popPair(matchSequence[searchDepth]);
	}
	inline void pushOperation(const NodeIDType& query_id, const NodeIDType& target_id) {
		state.pushPair(query_id, target_id);
		searchDepth++;
	};
	size_t threadPoolAllowBifurcationNum() {
		size_t working_num = thread_pool->runningThreadNum();
		size_t idle_num = thread_pool->threadNum() - thread_pool->runningThreadNum();
		size_t task_rest_num = thread_pool->restTaskNum();
		if (idle_num < task_rest_num)return 0;
		else return max<size_t>(1, idle_num / working_num);
	}
	inline void prepareMinState() {
		auto head = min_state.depth();
		LOOP(i, head, minDepth) {
			min_state.pushPair(matchSequence[i], target_sequence[i]);
		}
	}
	void task() {
		bool ok = false;
		auto prepared_unit = prepare_units->pop(ok);
		assert(ok == true);
		prepared_unit->run();
		free_units->push(move(prepared_unit));
		return;
	}
	void bifurcation(size_t num) {
		if (num == 0)return;
		num++;
		prepareMinState();
		auto& min_depth_first = cand_id[minDepth].first;
		auto min_depth_last_num = cand_id[minDepth].second - min_depth_first;
		if (min_depth_last_num == 1)return;//if min depth has no canditate target node,I have to calculate min_depth and min_state again.
		bool ok = false;
		while (num > 1 && min_depth_last_num > 1) {
			auto free_unit = free_units->pop(ok);
			if (ok == false) return;
			size_t this_unit_need = (min_depth_last_num / num) + (((min_depth_last_num % num) != 0) ? 1 : 0);
			assert(this_unit_need != min_depth_last_num);
			SIUnit* siu = free_unit.get();
			prepare(siu);
			siu->cand_id[minDepth].first = siu->cand_id[minDepth].second - min_depth_last_num;
			siu->cand_id[minDepth].second = siu->cand_id[minDepth].first + this_unit_need;
			prepare_units->push(move(free_unit));
			thread_pool->addTask(&SIUnit::task, this);

			min_depth_last_num -= this_unit_need;
			min_depth_first += this_unit_need;
			--num;
		}
		return;
	}
	void prepare(SIUnit* siu) {
		siu->state = min_state;
		siu->min_state = min_state;
		siu->minDepth = minDepth;
		siu->searchDepth = minDepth;
		std::copy(target_sequence.begin(), target_sequence.begin() + minDepth, (siu->target_sequence).begin());
		siu->state.calCandidatePairs(matchSequence[minDepth], siu->cand_id[minDepth].first, siu->cand_id[minDepth].second);

	}
	void run_no_recursive() {
		auto last_check_spilt_time = clock();
		size_t allow_bifurcate_depth = (minDepth + maxDepth) / 2;
		while (true) {
			if (end == true)return;
			if (clock() - last_check_spilt_time > (CLOCKS_PER_SEC / 10)) {
				auto bifurcation_num = threadPoolAllowBifurcationNum();
				if (minDepth > allow_bifurcate_depth) 	allow_bifurcate_depth = minDepth;
				else bifurcation(bifurcation_num);
				last_check_spilt_time = clock();

			}

			while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
				const auto query_id = matchSequence[searchDepth], target_id = *cand_id[searchDepth].first;
				cand_id[searchDepth].first++;
				if (!state.checkPair(query_id, target_id)) continue;
				pushOperation(query_id, target_id);
				if (searchDepth == minDepth && cand_id[searchDepth].first == cand_id[searchDepth].second) {
					target_sequence[searchDepth] = target_id;
					++searchDepth;
				}
				if (searchDepth == maxDepth) {	//find a solution, just pop last pair ,it will not effect the correction of answer;
					ToDoAfterFindASolution();
					if (needOneSolution) {
						end = true;
						return;
					}
					popOperation();
				}
				else	state.calCandidatePairs(matchSequence[searchDepth], cand_id[searchDepth].first, cand_id[searchDepth].second);
			}
			//add assert
			if (searchDepth == 0)return;
			else popOperation();
		}
	}
public:
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, vector<NodeIDType>& _mS, bool _oneSolution, shared_ptr<stack_mutex<unique_ptr<SIUnit>>> _free_siunits,
		shared_ptr<stack_mutex<unique_ptr<SIUnit>>> _prepare_units, atomic_bool& _end, shared_ptr<SubgraphMatchState<GraphType>[]> _sp, shared_ptr<ThreadPool> _tp) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _mS, _oneSolution), answerReceiver(_answerReceiver), maxDepth(_q.size()), free_units(_free_siunits),
		state(_q, _t, _sp), end(_end), cand_id(_q.size()), thread_pool(_tp), prepare_units(_prepare_units)
	{
		min_state = state;
		target_sequence.resize(_q.size(), NO_MAP);
	}

	void prepare(const NodeIDType query_id, const NodeIDType target_id) {
		if (state.checkPair(query_id, target_id) == false) {
			cand_id[1].first = cand_id[1].second = nullptr;
			return;
		}
		state.pushPair(query_id, target_id);
		state.calCandidatePairs(matchSequence[1], cand_id[1].first, cand_id[1].second);
		searchDepth = 1;
		minDepth = 1;
		assert(min_state.depth() != 0);
		//min_state = state;
		target_sequence[0] = target_id;
		return;
	}

	void run() {
		if (cand_id[searchDepth].first == cand_id[searchDepth].second) return;
		run_no_recursive();
	}

};

template<typename GraphType, typename AnswerReceiverType, typename MatchOrderSelector = MatchOrderSelectorVF3<GraphType> >
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase<GraphType, MatchOrderSelector> {
	typedef size_t NodeIDType;
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	AnswerReceiverType& answerReceiver;
	size_t threadNum;
	condition_variable work_cv;
	shared_ptr<SubgraphMatchState<GraphType>[]> subgraphStates;
	State<GraphType> state;

	atomic_bool end = false;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t _threadN,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), threadNum(_threadN)
	{
		subgraphStates = makeSubgraphState<GraphType>(_queryGraph, matchSequence);
		state = State<GraphType>(*queryGraphPtr, *targetGraphPtr, subgraphStates);
	}
	void run() {
		const NodeIDType* begin_ptr = nullptr, * end_ptr = nullptr;
		auto free_units = make_shared<stack_mutex<unique_ptr<SIUnit>>>();
		auto prepare_units = make_shared<stack_mutex<unique_ptr<SIUnit>>>();
		shared_ptr<ThreadPool> thread_pool = make_shared<ThreadPool>(threadNum);
		state.calCandidatePairs(matchSequence[0], begin_ptr, end_ptr);
		auto tasksDistribute = [&](const NodeIDType query_id, const NodeIDType target_id) {
			if (end)return;
			bool ok = false;
			auto freeUnit = move(free_units->pop(ok));
			if (!ok)return;
			freeUnit->prepare(query_id, target_id);
			freeUnit->run();
			free_units->push(move(freeUnit));
		};

		LOOP(i, 0, threadNum * 2) {
			thread_pool->addTask([&]() {
				auto p = make_unique<SIUnit>(*queryGraphPtr, *targetGraphPtr, answerReceiver, matchSequence, needOneSolution, free_units, prepare_units, end, subgraphStates, thread_pool);
				free_units->push(move(p));
				});
		}
		const auto query_id = matchSequence[0];
		while (begin_ptr != end_ptr) {
			thread_pool->addTask(tasksDistribute, query_id, *begin_ptr);
			++begin_ptr;
		}

		thread_pool->join();
		return;
	}

};
}