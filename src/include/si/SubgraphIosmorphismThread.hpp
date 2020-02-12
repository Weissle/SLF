#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIosmorphism.hpp"
#include"State.hpp"
#include"graph/Graph.hpp"
#include"MatchOrderSelector.hpp"
namespace wg {
template<typename GraphType, typename AnswerReceiverType>
class SubgraphIsomorphismThreadUnit {

};

template<typename GraphType, typename AnswerReceiverType, typename _MatchOrderSelector = void >
class SubgraphIsomorphismThread : public SubgraphIsomorphism<GraphType>{
//	typedef SubgraphIsomorphismThreadUnit<GraphType, AnswerReceiverType> SIUnit;
	vector<NodeIDType> matchSequence;
	StateType mapState;
	AnswerReceiverType& answerReceiver;
	bool induceGraph;
	bool onlyNeedOneSolution;
	size_t threadNum;

public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(GraphType& _queryGraph,GraphType& _targetGraph, AnswerReceiverType& _answerReceiver,size_t _threadN, bool _induceGraph = true,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:SubgraphIsomorphism<GraphType>(_queryGraph,_targetGraph), answerReceiver(_answerReceiver), induceGraph(_induceGraph), onlyNeedOneSolution(_onlyNeedOneSolution), mapState(_queryGraph, _targetGraph),threadNum(_threadN) {
		if (_matchSequence.size() == 0) {
			auto t1 = clock();
			if (typeid(_MatchOrderSelector) != typeid(void))matchSequence = _MatchOrderSelector::run(_queryGraph, _targetGraph);
			else matchSequence = MatchOrderSelectorVF3<GraphType>::run(_queryGraph, _targetGraph);
			TIME_COST_PRINT("match order selete time : ", clock() - t1);
			TRAVERSE_SET(nodeID, matchSequence) cout << nodeID << " ";
			cout << endl;
		}
		else matchSequence = _matchSequence;
	}
	void run() {
/*		SIUnit si1(queryGraph, targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution, matchSequence);
		si1 =move( SIUnit(queryGraph, targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution, matchSequence));*/
	}

};

}