#pragma once
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include"ThreadRelatedClass.hpp"
#include"SearchTree.hpp"
#include<si/MatchOrderSelector.hpp>
#include<vector>
#include<iostream>
#include<time.h>
#include<fstream>
#include<typeinfo>
#include<utility>

//#define DETAILS_TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , SubgraphIsomorphism will use default MatchOrderSelector.

*/
namespace wg {
template<class GraphType, class MatchOrderSelectorType = MatchOrderSelectorVF3<GraphType>>
class SubgraphIsomorphismBase {
protected:
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename GraphType::EdgeType EdgeType;

	typedef State<GraphType> StateType;
	const GraphType* targetGraphPtr, * queryGraphPtr;
	vector<NodeIDType> matchSequence;
	bool needOneSolution;
public:
	SubgraphIsomorphismBase() = default;
	SubgraphIsomorphismBase(const GraphType& _q,const GraphType& _t, const vector<NodeIDType>& _mS, bool needOS = false) :queryGraphPtr(&_q), targetGraphPtr(&_t), matchSequence(_mS), needOneSolution(needOS)
	{
		auto t1 = clock();
		if (matchSequence.size() == 0) 	matchSequence = MatchOrderSelectorType::run(_q, _t);

	}
};
//for single thread
template<typename GraphType, typename AnswerReceiverType, typename _MatchOrderSelector = void >
class SubgraphIsomorphism : public SubgraphIsomorphismBase<GraphType, _MatchOrderSelector> {

protected:
	size_t searchDepth;
	StateType mapState;
	AnswerReceiverType& answerReceiver;
	bool goDeeper_timeCount()
	{
		if (searchDepth == queryGraph.size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
		++hitTime;
		if ((int)hitTime % (int)1E4 == 0) {
			cout << hitTime << endl;
		}
		auto t1 = clock();
		const auto canditatePairs = move(mapState.calCandidatePairs(matchSequence[searchDepth]));
		auto t2 = clock();
		cal += t2 - t1;
		if (canditatePairs.empty())return false;
		canditatePairCount += canditatePairs.size();

		for (const auto& tempCanditatePair : canditatePairs) {
			t1 = clock();
			const bool suitable = mapState.checkPair(tempCanditatePair);
			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				mapState.pushPair(tempCanditatePair);
				++searchDepth;
				t2 = clock();
				add += t2 - t1;
				if (goDeeper_timeCount() && this->needOneSolution) return true;
				t1 = clock();
				mapState.popPair(tempCanditatePair);
				--searchDepth;
				t2 = clock();
				del += t2 - t1;
			}
		}
		return false;
	}
	bool goDeeper() {
		if (searchDepth == queryGraph.size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
		const auto& canditatePairs = mapState.calCandidatePairs(matchSequence[searchDepth]);
		for (const auto& tempCanditatePair : canditatePairs) {
			const bool suitable = mapState.checkPair(tempCanditatePair);
			if (suitable) {
				mapState.pushPair(tempCanditatePair);
				++searchDepth;
				if (goDeeper() && this->needOneSolution) return true;
				mapState.popPair(tempCanditatePair);
				--searchDepth;
			}
		}

		return false;
	}
	inline void ToDoAfterFindASolution() {
		answerReceiver << mapState.getMap();
	}
	void run_no_recursive() {

		const auto queryGraphSize = queryGraphPtr->size();
		SearchTree searchTree(queryGraphSize);
		searchTree.setTree(0, move(mapState.calCandidatePairs(matchSequence[0])));

		auto popOperation = [&]() {
			searchDepth--;
			mapState.popPair(matchSequence[searchDepth]);
		};
		auto pushOperation = [&](MapPair& p) {
			mapState.pushPair(p);
			searchDepth++;
		};
		while (true) {
			while (searchTree.empty(searchDepth) == false) {
				auto tempPair = searchTree.pop(searchDepth);
				if (!mapState.checkPair(tempPair)) continue;
				pushOperation(tempPair);
				if (searchDepth == queryGraphSize) {	//find a solution, just pop last pair ,it will not effect the correction of answer;
					ToDoAfterFindASolution();
					if (needOneSolution) return;
					popOperation();
				}
				else	searchTree.setTree(searchDepth, move(mapState.calCandidatePairs(matchSequence[searchDepth])));
			}
			if (searchDepth == 0)return;
			else popOperation();
		}
	}
	size_t cal = 0, check = 0, add = 0, del = 0, hitTime = 0;
	long long canditatePairCount = 0;
private:

public:

	SubgraphIsomorphism() = default;
	~SubgraphIsomorphism() = default;
	SubgraphIsomorphism(const GraphType& _queryGraph,const GraphType& _targetGraph,AnswerReceiverType& _answerReceiver, bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, _MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph, matchSequence)
	{
#ifdef OUTPUT_MATCH_SEQUENCE
		TRAVERSE_SET(nodeID, matchSequence) cout << nodeID << " ";
		cout << endl;
#endif
		searchDepth = 0;
};
	void run()
	{
#ifdef DETAILS_TIME_COUNT
		cout << "start match" << endl;
		goDeeper_timeCount();
		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;
#else
		//	goDeeper();
		run_no_recursive();
#endif
}

};

}