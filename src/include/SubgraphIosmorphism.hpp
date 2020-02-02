#pragma once
#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<map>
#include<fstream>
#include<MatchOrderSelector.hpp>
#include<typeinfo>
#include<utility>
#define TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , SubgraphIsomorphism will use default MatchOrderSelector.

*/
namespace wg {
template<typename StateType, typename _AnswerReceiver, typename _MatchOrderSelector = void >
class SubgraphIsomorphism {
	typedef _AnswerReceiver AnswerReceiver;
	typedef typename StateType::GraphType GraphType;
	typedef typename StateType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename StateType::EdgeType EdgeType;

	typedef typename StateType::MapType MapType;
	typedef typename StateType::MapPair MapPair;
	
	const GraphType& targetGraph, & queryGraph;
	vector<NodeIDType> matchSequence;
	size_t searchDepth;
	StateType mapState;
	AnswerReceiver& answerReceiver;
	unique_ptr< vector< MapPair>[] > allDepthCanditatePairs;

	bool onlyNeedOneSolution = true;
	bool induceGraph = true;
	bool goDeeper()
	{
		if (searchDepth==queryGraph.size()) {
			this->ToDoAfterFindASolution();
			return true;
		}
#ifdef TIME_COUNT
		++hitTime;
		if ((int)hitTime % (int)1E4 == 0) {
			cout << hitTime << endl;
		}
		auto t1 = clock();
	//	allDepthCanditatePairs[searchDepth] =mapState.calCandidatePairs(matchSequence[searchDepth]);
		allDepthCanditatePairs[searchDepth] = std::move(mapState.calCandidatePairs(matchSequence[searchDepth]));
		const auto& canditatePairs = allDepthCanditatePairs[searchDepth];
		auto t2 = clock();
		cal += t2 - t1;
		if (canditatePairs.empty())return false;
		//	cout << canditatePairs.size() << ' ';
		canditatePairCount += canditatePairs.size();


		LOOP(i,0,canditatePairs.size()){
			const auto tempCanditatePair = canditatePairs[i];
			const auto queryNodeID = tempCanditatePair.first;
			const auto targetNodeID = tempCanditatePair.second;
			t1 = clock();
			bool suitable = mapState.checkCanditatePairIsAddable(tempCanditatePair);

			t2 = clock();
			check += t2 - t1;
			if (suitable) {
				t1 = clock();
				mapState.addCanditatePairToMapping(tempCanditatePair);
				++searchDepth;
				t2 = clock();
				add += t2 - t1;
				if (goDeeper() && this->onlyNeedOneSolution) return true;
				t1 = clock();
				mapState.deleteCanditatePairToMapping(tempCanditatePair);
				--searchDepth;
				t2 = clock();
				del += t2 - t1;
			}
		}

#else
		if (mapState.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution(mapState);
			return true;
		}
		for (const auto& tempCanditatePair : mapState.calCandidatePairs(matchSequence[searchDepth])) {
			if (mapState.checkCanditatePairIsAddable(tempCanditatePair)) {
				mapState.addCanditatePairToMapping(tempCanditatePair);
				if (goDeeper(mapState) && this->onlyNeedOneSolution) return true;
				mapState.deleteCanditatePairToMapping(tempCanditatePair);
			}
		}
#endif
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
	SubgraphIsomorphism(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiver& _answerReceiver, bool _induceGraph = true, bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence= vector<NodeIDType>())
		:targetGraph(_targetGraph), queryGraph(_queryGraph), matchSequence(_matchSequence), onlyNeedOneSolution(_onlyNeedOneSolution), induceGraph(_induceGraph), answerReceiver(_answerReceiver), mapState(_queryGraph, _targetGraph)
	{
		auto t1 = clock();
		if (matchSequence.size() == 0) {
			if (typeid(_MatchOrderSelector) != typeid(void))matchSequence = _MatchOrderSelector::run(_queryGraph, _targetGraph);
			else matchSequence = MatchOrderSelectorVF3<GraphType>::run(_queryGraph, _targetGraph);
		}
		TIME_COST_PRINT("match order selete time : ", clock() - t1);
		TRAVERSE_SET(nodeID, matchSequence) cout << nodeID << " ";
		cout << endl;
		searchDepth = 0;
		allDepthCanditatePairs = std::move(unique_ptr<vector<MapPair>[]>(new vector<MapPair>[queryGraph.size()]));


	};
	void run()
	{
		cout << "start match" << endl;
		goDeeper();
	
		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		cout << "hit times " << hitTime << endl;
		cout << "canditate Pair Count " << canditatePairCount << endl;
	}

};

}