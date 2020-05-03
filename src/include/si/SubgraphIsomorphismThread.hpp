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
#include"tools/ThreadPool.hpp"
#include"si/TaskDistributor.hpp"
#include<atomic>
#include<time.h>
mutex m;
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	AnswerReceiverType& answerReceiver;
	size_t searchDepth = 0, maxDepth = 0;
	State<GraphType> state;

	size_t minDepth;
	State<GraphType> min_state;
	vector<NodeIDType> target_sequence;
	DynamicArray<pair<const NodeIDType*, const NodeIDType*>> cand_id;
	shared_ptr<TaskDistributor<SIUnit>> task_distributor;
	inline void ToDoAfterFindASolution() {
		answerReceiver << state.getMap();
	}
	inline void popOperation() {
		searchDepth--;
		state.popPair((*match_sequence_ptr)[searchDepth]);
	}
	inline void pushOperation(const NodeIDType& query_id, const NodeIDType& target_id) {
		state.pushPair(query_id, target_id);
		searchDepth++;
	};

	inline void prepareMinState() {
		auto head = min_state.depth();
		LOOP(i, head, minDepth) {
			min_state.pushPair((*match_sequence_ptr)[i], target_sequence[i]);
		}
	}

	void bifurcation(size_t num) {
		if (num == 0)return;
		num++;
		prepareMinState();
		auto& min_depth_first = cand_id[minDepth].first;
		auto min_depth_last_num = cand_id[minDepth].second - min_depth_first;
		bool ok = false;
		//if min depth has no canditate target node,I have to calculate min_depth and min_state again.
		while (num > 1 && min_depth_last_num > 1) {
			auto free_unit = move(task_distributor->getFreeUnit(ok));
			if (ok == false) return;
			//	cout << 1;
			size_t this_unit_need = (min_depth_last_num / num) + (((min_depth_last_num % num) != 0) ? 1 : 0);
			assert(this_unit_need != min_depth_last_num && this_unit_need != 0);
			SIUnit* siu = free_unit.get();
			prepare(siu);
			siu->cand_id[minDepth].first = siu->cand_id[minDepth].second - min_depth_last_num;
			siu->cand_id[minDepth].second = siu->cand_id[minDepth].first + this_unit_need;
			assert((siu->cand_id[minDepth].second - siu->cand_id[minDepth].first) + (cand_id[minDepth].second - cand_id[minDepth].first) == min_depth_last_num);
			task_distributor->addPreparedUnit(move(free_unit));
			min_depth_first += this_unit_need;
			/*		if (((siu->cand_id[minDepth].second - siu->cand_id[minDepth].first) + (cand_id[minDepth].second - cand_id[minDepth].first)) != min_depth_last_num) {
						m.lock();
						cout << 3 << endl;
						cout << (siu->cand_id[minDepth].second - siu->cand_id[minDepth].first) << endl;
						cout << (cand_id[minDepth].second - cand_id[minDepth].first) << endl;
						cout << min_depth_last_num << endl;
						cout << 2 << endl;
						m.unlock();
					}*/
			min_depth_last_num -= this_unit_need;
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
		siu->state.calCandidatePairs((*match_sequence_ptr)[minDepth], siu->cand_id[minDepth].first, siu->cand_id[minDepth].second);
	}
	void run_no_recursive() {
		size_t allow_bifurcate_depth = (minDepth + maxDepth) / 2;
		while (true) {
			auto query_id = (*match_sequence_ptr)[searchDepth];
	/*		if (task_distributor->end == true)return;
			if (task_distributor->allowDistribute()) {
				if (minDepth > allow_bifurcate_depth) 	allow_bifurcate_depth = minDepth;
				else {
					auto bifurcation_num = 1;// threadPoolAllowBifurcationNum();
					bifurcation(bifurcation_num);
				}

			}*/

			while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
				const auto target_id = *cand_id[searchDepth].first;
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
						task_distributor->end = true;
						return;
					}
					popOperation();
				}
				else {
					query_id = (*match_sequence_ptr)[searchDepth];
					state.calCandidatePairs(query_id, cand_id[searchDepth].first, cand_id[searchDepth].second);
				}
			}
			if (searchDepth == 0)return;
			else if (searchDepth <= minDepth && task_distributor->distribute_period == true)return;
			else popOperation();
		}
	}
public:
	SubgraphIsomorphismThreadUnit(const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, shared_ptr<const vector<NodeIDType>> _msp, bool _oneSolution,
		shared_ptr<const SubgraphMatchState<GraphType>[]> _sp, shared_ptr<TaskDistributor<SIUnit>> _tc) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _msp, _oneSolution), answerReceiver(_answerReceiver), maxDepth(_q.size()),
		state(_q, _t, _sp), cand_id(_q.size()), task_distributor(_tc)
	{
		min_state = state;
		target_sequence.resize(_q.size(), NO_MAP);
	}

	void prepare(const size_t min_depth_, const size_t dis_from_end, const size_t num) {

		state.calCandidatePairs((*match_sequence_ptr)[min_depth_], cand_id[min_depth_].first, cand_id[min_depth_].second);
		cand_id[min_depth_].first = cand_id[min_depth_].second - dis_from_end;
		cand_id[min_depth_].second = cand_id[min_depth_].first + num;
		minDepth = min_depth_;
		searchDepth = min_depth_;
		assert(min_state.depth() == 0);
	/*	m.lock();
		for (auto i = cand_id[min_depth_].first; i < cand_id[min_depth_].second; ++i) {
			cout << *i << ' ';
		}
		cout << endl;
		m.unlock();*/
		//cout << cand_id[min_depth_].first << ' ' << cand_id[min_depth_].second << endl;
		return;
	}

	void run() {
		if (cand_id[searchDepth].first == cand_id[searchDepth].second) return;
		run_no_recursive();
	}

};

template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase<GraphType> {
	typedef size_t NodeIDType;
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	AnswerReceiverType& answerReceiver;
	size_t threadNum;
	shared_ptr<const SubgraphMatchState<GraphType>[]> subgraphStates;
	State<GraphType> state;

public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t _threadN,
		bool _onlyNeedOneSolution, shared_ptr<const vector<NodeIDType>>& _match_sequence_ptr)
		:SubgraphIsomorphismBase<GraphType>(_queryGraph, _targetGraph, _match_sequence_ptr, _onlyNeedOneSolution), answerReceiver(_answerReceiver), threadNum(_threadN)
	{
		subgraphStates = makeSubgraphState<GraphType>(_queryGraph, match_sequence_ptr);
		state = State<GraphType>(*queryGraphPtr, *targetGraphPtr, subgraphStates);
	}
	void run() {
		const NodeIDType* begin_ptr = nullptr, * end_ptr = nullptr;
		const auto query_id = (*match_sequence_ptr)[0];
		auto task_distributor = make_shared<TaskDistributor<SIUnit>>(threadNum);
		state.calCandidatePairs(query_id, begin_ptr, end_ptr);
		auto tasksDistribute = [&](const size_t dis_from_end, const size_t num) {
			if (task_distributor->end)return;
			bool ok = false;
			auto freeUnit = move(task_distributor->getFreeUnit(ok));
			assert(ok);
			freeUnit->prepare(0, dis_from_end, num);
			freeUnit->run();
			task_distributor->addFreeUnit(move(freeUnit));
		};
		LOOP(i, 0, threadNum + 1) {
			task_distributor->addTask([&]() {
				auto p = make_unique<SIUnit>(*queryGraphPtr, *targetGraphPtr, answerReceiver, match_sequence_ptr, needOneSolution, subgraphStates, task_distributor);
				task_distributor->addFreeUnit(move(p));
				});
		}
		if (end_ptr - begin_ptr != targetGraphPtr->size()) { cout << 1; }
		/*	while (begin_ptr != end_ptr) {
				task_distributor->addTask(tasksDistribute, query_id, *begin_ptr);
				++begin_ptr;
			}
	*/
		size_t spilt = threadNum << 2;
		size_t cand_node_num = end_ptr - begin_ptr;
		size_t each_least = cand_node_num / spilt;
		size_t mod_more = cand_node_num % spilt;
		size_t temp = 0;
		for (auto i = 0; i < spilt; ++i) {
			size_t this_have = each_least;
			if (mod_more) {
				this_have++;
				mod_more--;
			}
			temp += this_have;
			cout << temp << endl;
			task_distributor->addTask(tasksDistribute, cand_node_num, this_have);
			cand_node_num -= this_have;
		}
		task_distributor->join();
		return;
	}

};
}