#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIsomorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"MatchOrderSelector.hpp"
#include"ThreadRelatedClass.hpp"
#include<utility>
#include<memory>
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	AnswerReceiverType& answerReceiver;
	size_t searchDepth = 0, maxDepth = 0;
	State<GraphType> state;
	size_t id;
	vector_mutex& freeThreads;
	condition_variable& cv;
	vector<NodeIDType> targetGraphMapSequence;
	volatile bool& end;
	void ToDoAfterFindASolution() {
		answerReceiver << state.getMap();
	}
	bool goDeeper() {
		if (searchDepth == maxDepth) {
			if (needOneSolution) end = true;
			this->ToDoAfterFindASolution();
			return true;
		}
		if (searchTree.empty(searchDepth))searchTree.setTree(searchDepth, move(state.calCandidatePairs(matchSequence [searchDepth])));
		while (searchTree.empty(searchDepth) == false) {
			const auto tempCanditatePair = searchTree.pop(searchDepth);
			const bool suitable = state.checkPair(tempCanditatePair);
			if (suitable) {
				state.pushPair(tempCanditatePair);
				++searchDepth;
				if (end || (goDeeper() && this->needOneSolution)) {
					return true;
				}
				state.popPair(tempCanditatePair);
				--searchDepth;
			}
		}

		return false;
	}

public:
	SubgraphIsomorphismThreadUnit(size_t _id, GraphType& _q, GraphType& _t, AnswerReceiverType& _answerReceiver, vector<NodeIDType>& _mS, bool _oneSolution, vector_mutex& _freeThreads,
		condition_variable& _cv, bool& _end,shared_ptr<GraphMatchState<GraphType>[]> _sp) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _mS, _oneSolution), id(_id), answerReceiver(_answerReceiver), maxDepth(_q.size()), 
		state(_q, _t,_sp), freeThreads(_freeThreads), cv(_cv), end(_end)
	{
		targetGraphMapSequence.resize(_q.size());
		for (auto& it : targetGraphMapSequence) it = NO_MAP;
	}

	void prepare(State<GraphType>& s, size_t _nD, vector<MapPair>& ps) {
		this->state = s;
		this->searchDepth = _nD;
		searchTree.setTree(_nD, ps);
	}
	void run() {
		auto t1 = clock();
		goDeeper();
		freeThreads.push_back(id);
		cv.notify_one();
		PRINT_TIME_COST_S(to_string(id) + " :", clock() - t1);
	}
	pair<size_t, size_t> minDepth_and_restPair() {
		return move(searchTree.minDepth_and_restPair());
	}

};

template<typename GraphType, typename AnswerReceiverType, typename MatchOrderSelector = MatchOrderSelectorVF3<GraphType> >
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase<GraphType, MatchOrderSelector> {
	typedef size_t NodeIDType;
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	State<GraphType> state;
	AnswerReceiverType& answerReceiver;
	size_t threadNum;
	vector<SIUnit> siUnits;
	vector<thread> threads;
	vector_mutex freeThreads, runningThreads;
	condition_variable work_cv;
	shared_ptr<GraphMatchState<GraphType>[]> subgraphStates;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(GraphType& _queryGraph, GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t _threadN,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver),
		threadNum(_threadN)
	{
		subgraphStates =State<GraphType>::makeSubgraphState(_queryGraph, matchSequence);
		state = State<GraphType>(queryGraph, targetGraph, subgraphStates);
		threads.resize(_threadN);
	}
	void run() {
		bool end = false;
		LOOP(i, 0, threadNum) {
			siUnits.push_back(SIUnit(i, queryGraph, targetGraph, answerReceiver, matchSequence, needOneSolution, freeThreads, work_cv, end, subgraphStates));
		}
		vector<pair<NodeIDType, NodeIDType>> tempCPs = state.calCandidatePairs(matchSequence[0]);
		vector<vector<pair<NodeIDType, NodeIDType>>> vvp(threadNum);
		{
			size_t oneSIhave = tempCPs.size() / threadNum;
			auto it = tempCPs.begin();
			LOOP(i, 0, threadNum - 1) {
				vvp[i].assign(it, it + oneSIhave);
				it += oneSIhave;
			}
			vvp[threadNum - 1].assign(it, tempCPs.end());
		}
		LOOP(i, 0, threadNum) siUnits[i].prepare(state, 0, vvp[i]);
		LOOP(i, 0, threadNum) threads[i] = thread(&SIUnit::run, &siUnits[i]);
		LOOP(i, 0, threadNum) threads[i].join();
		return;
	}

private:
};
}