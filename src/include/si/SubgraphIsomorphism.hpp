#pragma once
#include"State.hpp"
#include"AnswerReceiver.hpp"
#include "graph/Graph.hpp"
#include"si/si_marcos.h"
#include"si/Tasks.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<utility>

//#define DETAILS_TIME_COUNT
using namespace std;

namespace wg {

//for single thread
template<typename EdgeLabel>
class SequentialSubgraphIsomorphism {
	using GraphType = GraphS<EdgeLabel>;
protected:
	vector<NodeIDType> matchSequence;
	size_t _limits; //how many _limits you need , _limits == 0 means no _limits;
	size_t searchDepth;
	State<EdgeLabel> state;
	AnswerReceiver *answerReceiver;
	vector<Tasks<EdgeLabel>> cand_id;
	bool end = false;
	void sequentialSearch_timeCount(const int searchDepth)
	{
		const auto next_depth = searchDepth + 1;
		if (state.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution();
			return;
		}
		//++hitTime;
		const auto query_id = matchSequence[searchDepth];
		auto t1 = clock();
		state.calCandidatePairs(query_id, cand_id[searchDepth]);
		auto t2 = clock();
		cal += t2 - t1;
		canditatePairCount += cand_id[searchDepth].size();

		while (cand_id[searchDepth].size()) {
			t1 = clock();
			const auto target_id = cand_id[searchDepth].getTask();
			const bool suitable = state.AddAble(query_id, target_id);
			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				state.AddPair(query_id, target_id);
				t2 = clock();
				add += t2 - t1;
				sequentialSearch_timeCount(next_depth);
				t1 = clock();
				state.RemovePair(query_id);
				t2 = clock();
				if (end)return;
				del += t2 - t1;
			}
		}
	}

	inline void ToDoAfterFindASolution() {
		*answerReceiver << state.GetMapping();
		if ( _limits && (*answerReceiver).solutionsCount() >= _limits) end = true;
	}
	void sequentialSearch(const int searchDepth)
	{
		const auto search_depth = searchDepth;
		const auto next_depth = searchDepth + 1;
		if (state.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution();
			return;
		}

		const auto query_id = matchSequence[searchDepth];
		state.calCandidatePairs(query_id, cand_id[search_depth]);
		while (cand_id[search_depth].size()) {
			const auto target_id = cand_id[search_depth].getTask();
			if (state.AddAble(query_id, target_id)) {
				state.AddPair(query_id, target_id);
				sequentialSearch(next_depth);
				if(end)return;
				state.RemovePair(query_id);
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
	SequentialSubgraphIsomorphism(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiver *_answerReceiver, size_t __limits, const vector<NodeIDType>& _match_sequence)
		:matchSequence(_match_sequence),_limits(__limits),answerReceiver(_answerReceiver),
		state(_queryGraph, _targetGraph, makeSubgraphState(_queryGraph, _match_sequence)), cand_id(_queryGraph.Size()) {};
	void run()
	{
#ifdef DETAILS_TIME_COUNT
		sequentialSearch_timeCount(0);
		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		//cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;
#else
		sequentialSearch(0);
#endif
	}

	size_t callTimes()const { return this->hitTime; }

};

}
