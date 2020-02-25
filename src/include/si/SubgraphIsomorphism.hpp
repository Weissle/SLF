#pragma once
#include"graph/Edge.hpp"
#include"graph/Graph.hpp"
#include"graph/Node.hpp"
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<map>
#include<fstream>
#include<si/MatchOrderSelector.hpp>
#include<typeinfo>
#include<utility>
#define DETAILS_TIME_COUNT
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
	typedef typename StateType::MapType MapType;
	typedef typename StateType::MapPair MapPair;
	GraphType& targetGraph, & queryGraph;
	vector<NodeIDType> matchSequence;
	bool needOneSolution;
public:
	SubgraphIsomorphismBase() = default;
	SubgraphIsomorphismBase(GraphType& _q, GraphType& _t, const vector<NodeIDType>& _mS, bool needOS = false) :queryGraph(_q), targetGraph(_t), matchSequence(_mS), needOneSolution(needOS)
	{
		auto t1 = clock();
		if (matchSequence.size() == 0) 	matchSequence = MatchOrderSelectorType::run(_q, _t);

		/*
#ifdef DETAILS_TIME_COUNT
		TIME_COST_PRINT("match order selete time : ", clock() - t1);
#endif
	*/
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
			const bool suitable = mapState.checkCanditatePairIsAddable(tempCanditatePair);
			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				mapState.addCanditatePairToMapping(tempCanditatePair);
				++searchDepth;
				t2 = clock();
				add += t2 - t1;
				if (goDeeper_timeCount() && this->needOneSolution) return true;
				t1 = clock();
				mapState.deleteCanditatePairToMapping(tempCanditatePair);
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
			const bool suitable = mapState.checkCanditatePairIsAddable(tempCanditatePair);
			if (suitable) {
				mapState.addCanditatePairToMapping(tempCanditatePair);
				++searchDepth;
				if (goDeeper() && this->needOneSolution) return true;
				mapState.deleteCanditatePairToMapping(tempCanditatePair);
				--searchDepth;
			}
		}

		return false;
	}
	void ToDoAfterFindASolution() {
		answerReceiver << mapState.getMap();
	}
	size_t cal = 0, check = 0, add = 0, del = 0, hitTime = 0;
	long long canditatePairCount = 0;
private:

public:

	SubgraphIsomorphism() = default;
	~SubgraphIsomorphism() = default;
	SubgraphIsomorphism(GraphType& _queryGraph, GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType, _MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph,matchSequence)
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
		goDeeper();
#endif
	}

	};

}