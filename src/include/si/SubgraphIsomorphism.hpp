#pragma once
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include"ThreadRelatedClass.hpp"
#include<si/MatchOrderSelector.hpp>
#include<vector>
#include<iostream>
#include<time.h>
#include<fstream>
#include<typeinfo>
#include<utility>

#define DETAILS_TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , SubgraphIsomorphism will use default MatchOrderSelector.

*/
namespace wg {
template<class GraphType>
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
	SubgraphIsomorphismBase(const GraphType& _q, const GraphType& _t, const vector<NodeIDType>& _mS, bool needOS = false) :queryGraphPtr(&_q), targetGraphPtr(&_t), matchSequence(_mS), needOneSolution(needOS)
	{
		assert(_mS.size() == _q.size());

	}
};
//for single thread
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphism : public SubgraphIsomorphismBase<GraphType> {

protected:
	size_t searchDepth;
	StateType mapState;
	AnswerReceiverType& answerReceiver;
	DynamicArray<pair<const NodeIDType*, const NodeIDType*>> cand_id;
	bool goDeeper_timeCount()
	{
		if (searchDepth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
		++hitTime;
		if ((int)hitTime % (int)1E4 == 0) {
			//			cout << hitTime << endl;
		}
		const auto query_id = matchSequence[searchDepth];
		auto t1 = clock();
		mapState.calCandidatePairs(query_id, cand_id[searchDepth].first, cand_id[searchDepth].second);
		auto t2 = clock();
		cal += t2 - t1;
		canditatePairCount += cand_id[searchDepth].second - cand_id[searchDepth].first;

		while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
			t1 = clock();
			const auto target_id = *cand_id[searchDepth].first;
			cand_id[searchDepth].first++;
			const bool suitable = mapState.checkPair(query_id, target_id);
			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				mapState.pushPair(query_id, target_id);
				++searchDepth;
				t2 = clock();
				add += t2 - t1;
				if (goDeeper_timeCount() && this->needOneSolution) return true;
				t1 = clock();
				mapState.popPair(query_id);
				--searchDepth;
				t2 = clock();
				del += t2 - t1;
			}
		}
		return false;
	}

	inline void ToDoAfterFindASolution() {
		answerReceiver << mapState.getMap();
	}
	void run_no_recursive() {

		const auto queryGraphSize = queryGraphPtr->size();
		mapState.calCandidatePairs(matchSequence[0], cand_id[0].first, cand_id[0].second);
		auto popOperation = [&]() {
			searchDepth--;
			mapState.popPair(matchSequence[searchDepth]);
		};
		auto pushOperation = [&](const NodeIDType& query_id, const NodeIDType& target_id) {
			mapState.pushPair(query_id, target_id);
			searchDepth++;
		};

		while (true) {
			auto query_id = matchSequence[searchDepth];
			while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
				const auto target_id = *cand_id[searchDepth].first;
				cand_id[searchDepth].first++;
				if (!mapState.checkPair(query_id, target_id)) continue;
				pushOperation(query_id, target_id);
				if (searchDepth == queryGraphSize) {	//find a solution, just pop last pair ,it will not effect the correction of answer;
					ToDoAfterFindASolution();
					if (needOneSolution) return;
					popOperation();
				}
				else {
					query_id = matchSequence[searchDepth];
					mapState.calCandidatePairs(query_id, cand_id[searchDepth].first, cand_id[searchDepth].second);
				}
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
	SubgraphIsomorphism(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphismBase<GraphType>(_queryGraph, _targetGraph, _matchSequence, _onlyNeedOneSolution), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph,makeSubgraphState(_queryGraph,_matchSequence) ), cand_id(queryGraphPtr->size())

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
		goDeeper_timeCount();
/*		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;*/
#else
		run_no_recursive();
#endif
	}

	size_t callTimes()const { return this->hitTime; }

};

}