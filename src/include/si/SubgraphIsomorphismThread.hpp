#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIsomorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"MatchOrderSelector.hpp"
#include"ThreadRelatedClass.hpp"
#include"SearchTree.hpp"
#include<utility>
#include<memory>
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit : public SubgraphIsomorphismBase<GraphType> {
	AnswerReceiverType &answerReceiver;
	size_t searchDepth = 0, maxDepth = 0, minDepth = 0;
	State<GraphType> state,minState;
	size_t id;
	vector_mutex& freeThreads;
	condition_variable& finish_cv;
	vector<NodeIDType> targetGraphMapSequence;
	SearchTreeThread searchTree;
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
	void run_no_recursive() {
//		searchTree.setTree(0, move(state.calCandidatePairs(matchSequence[0])));
		const auto queryGraphSize = this->queryGraphPtr->size();
		auto popOperation = [&]() {
			searchDepth--;
			state.popPair(matchSequence[searchDepth]);
		};
		auto pushOperation = [&](MapPair& p) {
			state.pushPair(p);
			searchDepth++;
		};
		while (true) {
			if (end == true)return;
			while (searchTree.empty(searchDepth) == false) {
				auto tempPair = searchTree.pop(searchDepth);
				if (!state.checkPair(tempPair)) continue;
				pushOperation(tempPair);
				if (searchDepth == queryGraphSize) {	//find a solution, just pop last pair ,it will not effect the correction of answer;
					ToDoAfterFindASolution();
					if (needOneSolution) {
						end = true;
						return;
					}
					popOperation();
				}
				else	searchTree.setTree(searchDepth, move(state.calCandidatePairs(matchSequence[searchDepth])));
			}
			if (searchDepth == 0)return;
			else popOperation();
		}
	}
public:
	SubgraphIsomorphismThreadUnit(size_t _id,const GraphType& _q,const GraphType& _t, AnswerReceiverType &_answerReceiver, vector<NodeIDType>& _mS, bool _oneSolution, vector_mutex& _freeThreads,
		condition_variable& _cv, bool& _end,shared_ptr<GraphMatchState<GraphType>[]> _sp) :
		SubgraphIsomorphismBase<GraphType>(_q, _t, _mS, _oneSolution), id(_id), answerReceiver(_answerReceiver), maxDepth(_q.size()), 
		state(_q, _t,_sp), freeThreads(_freeThreads), finish_cv(_cv), end(_end),searchTree(_q.size())
	{
		targetGraphMapSequence.resize(_q.size());
		for (auto& it : targetGraphMapSequence) it = NO_MAP;
	}

	void prepare(MapPair p) {
		vector<MapPair> temp(1);
		temp[0] = p;
		searchTree.setTree(0, temp);
		return;
	}
	void prepare(State<GraphType>& s, size_t _nD, vector<MapPair>& ps) {
		this->state = s;
		this->searchDepth = _nD;
		searchTree.setTree(_nD, ps);
	}
	void run() {
	//	cout <<"unit "<< id << endl;
		auto t1 = clock();
//		goDeeper();
		run_no_recursive();
		lock_guard<mutex> lg(freeThreads.m);
		freeThreads.push_back(id);
		finish_cv.notify_one();
	//	PRINT_TIME_COST_S(to_string(id) + " :", clock() - t1);
	}
	pair<size_t, size_t> minDepth_and_restPair() {
		auto p = searchTree.minDepth_and_restPair();

		if (p.first == NO_MAP || maxDepth - p.first <= 30 || p.second==0)return pair<size_t, size_t>(NO_MAP, 0);
		return move(p);
	}

};

template<typename GraphType, typename AnswerReceiverType, typename MatchOrderSelector = MatchOrderSelectorVF3<GraphType> >
class SubgraphIsomorphismThread : public SubgraphIsomorphismBase<GraphType, MatchOrderSelector> {
	typedef size_t NodeIDType;
	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	AnswerReceiverType &answerReceiver;
	size_t threadNum;
	condition_variable work_cv;
	shared_ptr<GraphMatchState<GraphType>[]> subgraphStates;
	State<GraphType> state;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph,const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t _threadN,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver),threadNum(_threadN)
	{
		subgraphStates =State<GraphType>::makeSubgraphState(_queryGraph, matchSequence);
		state = State<GraphType>(*queryGraphPtr, *targetGraphPtr, subgraphStates);
	}
	void run() {
	
		bool end = false;
		const GraphType& targetGraph = *targetGraphPtr;
		const GraphType& queryGraph = *queryGraphPtr;
		vector<SIUnit> siUnits;
		vector_mutex freeThreads;
		vector<thread> threads(threadNum);

	/*	vector<pair<NodeIDType, NodeIDType>> tasks = state.calCandidatePairs(matchSequence[0]);
		auto tasksDistribute = [&](size_t freeUnit) {
	//		cout <<"main"<< freeUnit << endl;
			if (tasks.empty() == false) {
				if(threads[freeUnit].joinable()) threads[freeUnit].join();
				auto tempP = tasks.back();
				tasks.pop_back();
				siUnits[freeUnit].prepare(tempP);
				threads[freeUnit] = thread(&SIUnit::run, &siUnits[freeUnit]);
			}
		};
		LOOP(i, 0, threadNum) {
			siUnits.emplace_back(i, queryGraph, targetGraph, answerReceiver, matchSequence, needOneSolution, freeThreads, work_cv, end, subgraphStates);
	//		tasksDistribute(i);
			freeThreads.push_back(i);
		}
		while (tasks.size()) {
			unique_lock<mutex> ul(freeThreads.m);
			work_cv.wait(ul, [&freeThreads]() {return !freeThreads.empty(); });
			auto freeUnit = freeThreads.pop();
			ul.unlock();
			tasksDistribute(freeUnit);
		}

		LOOP(i, 0, threadNum) {
			if(threads[i].joinable())	threads[i].join();
		}*/

		LOOP(i, 0, threadNum) {
			siUnits.emplace_back(i, queryGraph, targetGraph, answerReceiver, matchSequence, needOneSolution, freeThreads, work_cv, end, subgraphStates);
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
		return;
	}

private:
};
}