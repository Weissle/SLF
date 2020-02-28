#pragma once
#include"graph/Edge.hpp"
#include"graph/Graph.hpp"
#include"graph/Node.hpp"
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include"ThreadRelatedClass.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<map>
#include<fstream>
#include<si/MatchOrderSelector.hpp>
#include<typeinfo>
#include<utility>
#include<stack>
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
	//	typedef typename StateType::MapType MapType;
	//	typedef typename StateType::MapPair MapPair;
	GraphType& targetGraph, & queryGraph;
	vector<NodeIDType> matchSequence;
	bool needOneSolution;
	SearchTree searchTree;
public:
	SubgraphIsomorphismBase() = default;
	SubgraphIsomorphismBase(GraphType& _q, GraphType& _t, const vector<NodeIDType>& _mS, bool needOS = false) :queryGraph(_q), targetGraph(_t), matchSequence(_mS), needOneSolution(needOS), searchTree(_q.size())
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
		:SubgraphIsomorphismBase<GraphType, _MatchOrderSelector>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph, matchSequence)
	{
#ifdef OUTPUT_MATCH_SEQUENCE
		TRAVERSE_SET(nodeID, matchSequence) cout << nodeID << " ";
		cout << endl;
#endif
		searchDepth = 0;
	};
	void run1()
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
	void run() {
		searchTree.setTree(0, move(mapState.calCandidatePairs(matchSequence[0])));
		const auto queryGraphSize = queryGraph.size();
		bool pop = false;
		auto popOperation = [&]() {
			searchDepth--;
			mapState.popPair(matchSequence[searchDepth]);
		};
		auto pushOperation = [&](MapPair& p) {
			mapState.pushPair(p);
			searchDepth++;
		};
		while (searchDepth <= queryGraphSize) {
			if (pop == false) {
				MapPair addPair(NO_MAP, NO_MAP);
				while (searchTree.empty(searchDepth) == false) {
					auto tempPair = searchTree.pop(searchDepth);
					if (mapState.checkPair(tempPair)) {
						addPair = tempPair;
						break;
					}
				}
				if (addPair.first == NO_MAP) {
					pop = true;
					continue;
				}
				else pushOperation(addPair);
				if (searchDepth == queryGraphSize) {
					ToDoAfterFindASolution();
					if (needOneSolution)break;
					popOperation();
				}
				else {
					searchTree.setTree(searchDepth, move(mapState.calCandidatePairs(matchSequence[searchDepth])));
				}
			}
			else {
				if (searchDepth == 0)break;
				else {
					pop = false;
					popOperation();
				}
			}
		}
		return;
	}

};

}