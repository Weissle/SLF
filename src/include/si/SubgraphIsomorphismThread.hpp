#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIsomorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"MatchOrderSelector.hpp"
#include"ThreadRelatedClass.hpp"
#include"SearchTree.hpp"
#include<mutex>
#include<utility>
#include<memory>
//#include<tools/ThreadPool.h>
#include<tools/DynamicThreadPool.hpp>
#include<atomic>
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	using SIUnit = SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType>;
	AnswerReceiverType& answerReceiver;
	size_t searchDepth = 0, maxDepth = 0, minDepth = 0;
	State<GraphType> state, minState;
	size_t id;
	stack_mutex<unique_ptr<SIUnit>>& freeThreads;
	condition_variable& finish_cv;
	vector<NodeIDType> targetGraphMapSequence;
	DynamicArray<pair<const NodeIDType*, const NodeIDType*>> cand_id;
	atomic_bool& end;
	inline void ToDoAfterFindASolution() {
		answerReceiver << state.getMap();
	}

	void run_no_recursive() {

		const auto queryGraphSize = queryGraphPtr->size();

		auto popOperation = [&]() {
			searchDepth--;
			state.popPair(matchSequence[searchDepth]);
		};
		auto pushOperation = [&](const NodeIDType& query_id, const NodeIDType& target_id) {
			state.pushPair(query_id, target_id);
			searchDepth++;
		};

		while (true) {
			if (end == true)return;
			while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
				const auto query_id = matchSequence[searchDepth], target_id = *cand_id[searchDepth].first;
				cand_id[searchDepth].first++;
				if (!state.checkPair(query_id, target_id)) continue;
				pushOperation(query_id, target_id);
				if (searchDepth == queryGraphSize) {	//find a solution, just pop last pair ,it will not effect the correction of answer;
					ToDoAfterFindASolution();
					if (needOneSolution) {
						end = true;
						return;
					}
					popOperation();
				}
				else	state.calCandidatePairs(matchSequence[searchDepth], cand_id[searchDepth].first, cand_id[searchDepth].second);
			}
			if (searchDepth == 0)return;
			else popOperation();
		}
	}
public:
	SubgraphIsomorphismThreadUnit(size_t _id, const GraphType& _q, const GraphType& _t, AnswerReceiverType& _answerReceiver, vector<NodeIDType>& _mS, bool _oneSolution, stack_mutex<unique_ptr<SIUnit>>& _freeThreads,
		condition_variable& _cv, atomic_bool& _end, shared_ptr<SubgraphMatchState<GraphType>[]> _sp) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _mS, _oneSolution), id(_id), answerReceiver(_answerReceiver), maxDepth(_q.size()),
		state(_q, _t, _sp), freeThreads(_freeThreads), finish_cv(_cv), end(_end),cand_id(_q.size())
	{
		targetGraphMapSequence.resize(_q.size());
		for (auto& it : targetGraphMapSequence) it = NO_MAP;
	}

	void prepare(const NodeIDType query_id, const NodeIDType target_id) {
		if (state.checkPair(query_id, target_id) == false) {
			cand_id[1].first = cand_id[1].second = nullptr;
		}
		else {
			state.pushPair(query_id, target_id);
			state.calCandidatePairs(matchSequence[1], cand_id[1].first, cand_id[1].second);
			searchDepth = 1;
		}
		return;
	}

	void run() {
		//		auto t1 = clock();
		run_no_recursive();
	}
	pair<size_t, size_t> minDepth_and_restPair() {
		auto p = searchTree.minDepth_and_restPair();
		if (p.first == NO_MAP || maxDepth - p.first <= 30 || p.second == 0)return pair<size_t, size_t>(NO_MAP, 0);
		return move(p);
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
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t _threadN,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), threadNum(_threadN)
	{
		subgraphStates = State<GraphType>::makeSubgraphState(_queryGraph, matchSequence);
		state = State<GraphType>(*queryGraphPtr, *targetGraphPtr, subgraphStates);
	}
	void run() {
		atomic_bool end = false;
		stack_mutex<unique_ptr<SIUnit>> freeUnits;
		vector<thread> threads(threadNum);
		const NodeIDType* begin_ptr = nullptr, * end_ptr = nullptr;
		DynamicThreadPool threadPool(threadNum);
		state.calCandidatePairs(matchSequence[0], begin_ptr, end_ptr);
		auto tasksDistribute = [&](const NodeIDType query_id, const NodeIDType target_id) {
			if (end)return;
			bool ok = false;
			auto freeUnit = move(freeUnits.pop(ok));
			if (!ok)return;
			freeUnit->prepare(query_id,target_id);
			freeUnit->run();
			freeUnits.push(move(freeUnit));
			work_cv.notify_one();
		};
		LOOP(i, 0, threadNum) {
			auto p = make_unique<SIUnit>(i, *queryGraphPtr, *targetGraphPtr, answerReceiver, matchSequence, needOneSolution, freeUnits, work_cv, end, subgraphStates);
			freeUnits.push(move(p));
		}
		while (begin_ptr!= end_ptr) {
			threadPool.addTask(tasksDistribute, matchSequence[0],*begin_ptr);
			++begin_ptr;
		}
		return;
	}

};
}