#pragma once
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include"si/si_marcos.h"
#include"si/Tasks.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<utility>

//#define DETAILS_TIME_COUNT
using namespace std;

namespace wg {

class SubgraphIsomorphismBase {
protected:
	shared_ptr<const vector<NodeIDType>> match_sequence_ptr;
	size_t _limits; //how many _limits you need , _limits == 0 means no _limits;
public:
	SubgraphIsomorphismBase() = default;
	SubgraphIsomorphismBase(const shared_ptr<const vector<NodeIDType>> _msp, size_t __limits ) : match_sequence_ptr(_msp), _limits(__limits)
	{
	}
};
//for single thread
template<typename GraphType, typename AnswerReceiverType>
class SequentialSubgraphIsomorphism : public SubgraphIsomorphismBase {
	const GraphType* queryGraphPtr, * targetGraphPtr;
	using EdgeLabelType = typename GraphType::EdgeLabelType;
protected:
	size_t searchDepth;
	State<GraphType> state;
	AnswerReceiverType& answerReceiver;
	vector<Tasks<EdgeLabelType>> cand_id;
	bool end = false;
	void sequentialSearch_timeCount()
	{

		if (searchDepth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return;
		}
		++hitTime;
		if ((int)hitTime % (int)1E4 == 0) {
			//			cout << hitTime << endl;
		}
		const auto query_id = (*match_sequence_ptr)[searchDepth];
		auto t1 = clock();
		state.calCandidatePairs(query_id, cand_id[searchDepth]);
		auto t2 = clock();
		cal += t2 - t1;
		canditatePairCount += cand_id[searchDepth].size();

		while (cand_id[searchDepth].size()) {
			t1 = clock();
			const auto target_id = cand_id[searchDepth].back();
			cand_id[searchDepth].pop_back();
			const bool suitable = state.AddAble(query_id, target_id);
			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				state.AddPair(query_id, target_id);
				++searchDepth;
				t2 = clock();
				add += t2 - t1;
				sequentialSearch_timeCount();
				t1 = clock();
				state.RemovePair(query_id);
				--searchDepth;
				t2 = clock();
				if (end)return;
				del += t2 - t1;
			}
		}
	}

	inline void ToDoAfterFindASolution() {
		answerReceiver << state.GetMapping();
		if ( _limits && answerReceiver.solutionsCount() >= _limits) end = true;
	}
	void sequentialSearch()
	{
		const auto search_depth = searchDepth;
		if (searchDepth == queryGraphPtr->size()) {
			this->ToDoAfterFindASolution();
			return;
		}

		const auto query_id = (*match_sequence_ptr)[searchDepth];
		state.calCandidatePairs(query_id, cand_id[search_depth]);
		while (cand_id[search_depth].size()) {
			const auto target_id = cand_id[search_depth].getTask();
			if (state.AddAble(query_id, target_id)) {
				state.AddPair(query_id, target_id);
				++searchDepth;
				sequentialSearch();
				if (end)return;
				state.RemovePair(query_id);
				--searchDepth;
			}
		}
		return;
	}
	size_t cal = 0, check = 0, add = 0, del = 0, hitTime = 0;
	long long canditatePairCount = 0;
private:

public:

	SequentialSubgraphIsomorphism() = default;
	~SequentialSubgraphIsomorphism() = default;
	SequentialSubgraphIsomorphism(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, size_t __limits, shared_ptr<const vector<NodeIDType>>& _match_sequence_ptr)
		:SubgraphIsomorphismBase(_match_sequence_ptr, __limits), answerReceiver(_answerReceiver),queryGraphPtr(&_queryGraph),targetGraphPtr(&_targetGraph),
		state(_queryGraph, _targetGraph, makeSubgraphState(_queryGraph, _match_sequence_ptr)), cand_id(queryGraphPtr->size())

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
		sequentialSearch_timeCount();
		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;
#else
		sequentialSearch();
#endif
	}

	size_t callTimes()const { return this->hitTime; }

};

}