#pragma once
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include"si/si_marcos.h"
#include<vector>
#include<iostream>
#include<time.h>
#include<utility>

//#define DETAILS_TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , SubgraphIsomorphism will use default MatchOrderSelector.

*/
namespace wg {
template<class GraphType>
class SubgraphIsomorphismBase {
protected:
	typedef typename GraphType::NodeType NodeType;
	typedef typename GraphType::EdgeType EdgeType;

	
	const GraphType* targetGraphPtr, * queryGraphPtr;
	shared_ptr<const vector<NodeIDType>> match_sequence_ptr;
	bool needOneSolution;
public:
	SubgraphIsomorphismBase() = default;
	SubgraphIsomorphismBase(const GraphType& _q, const GraphType& _t, const shared_ptr<const vector<NodeIDType>> _msp, bool needOS = false) :queryGraphPtr(&_q), targetGraphPtr(&_t), match_sequence_ptr(_msp), needOneSolution(needOS)
	{
		assert(_msp->size() == _q.size());
	}
};
//for single thread
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphism : public SubgraphIsomorphismBase<GraphType> {

protected:
	size_t searchDepth;
	State<GraphType> mapState;
	AnswerReceiverType& answerReceiver;
	vector<pair<const NodeIDType*, const NodeIDType*>> cand_id;
	bool run_timeCount()
	{

		if (searchDepth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
		++hitTime;
		if ((int)hitTime % (int)1E4 == 0) {
			//			cout << hitTime << endl;
		}
		const auto query_id = (*match_sequence_ptr)[searchDepth];
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
	bool run_noTimeCount()
	{
		if (searchDepth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return true;
		}

		const auto query_id = (*match_sequence_ptr)[searchDepth];
		mapState.calCandidatePairs(query_id, cand_id[searchDepth].first, cand_id[searchDepth].second);

		while (cand_id[searchDepth].first != cand_id[searchDepth].second) {
			const auto target_id = *cand_id[searchDepth].first;
			cand_id[searchDepth].first++;
			if (mapState.checkPair(query_id, target_id)) {
				mapState.pushPair(query_id, target_id);
				++searchDepth;
				if (run_noTimeCount() && this->needOneSolution) return true;
				mapState.popPair(query_id);
				--searchDepth;
			}
		}
		return false;
	}
	size_t cal = 0, check = 0, add = 0, del = 0, hitTime = 0;
	long long canditatePairCount = 0;
private:

public:

	SubgraphIsomorphism() = default;
	~SubgraphIsomorphism() = default;
	SubgraphIsomorphism(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, bool _onlyNeedOneSolution, shared_ptr<const vector<NodeIDType>>& _match_sequence_ptr)
		:SubgraphIsomorphismBase<GraphType>(_queryGraph, _targetGraph, _match_sequence_ptr, _onlyNeedOneSolution), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph,makeSubgraphState(_queryGraph,_match_sequence_ptr) ), cand_id(queryGraphPtr->size())

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
		run_timeCount();
/*		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;*/
#else
		run_noTimeCount();
#endif
	}

	size_t callTimes()const { return this->hitTime; }

};

}