#pragma once
#include<vector>
#include"AnswerReceiver.hpp"
#include"SubgraphIosmorphism.hpp"
#include"State.hpp"
#include"Graph.hpp"

namespace wg {

template<typename StateType, typename AnswerReceiverType, typename _MatchOrderSelector = void >
class SubgraphIsomorphismThread {
	typedef SubgraphIsomorphism<StateType, AnswerReceiverType, _MatchOrderSelector> SIUnit;
	typedef typename StateType::GraphType GraphType;
	typedef typename StateType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename StateType::EdgeType EdgeType;

	typedef typename StateType::MapType MapType;
	typedef typename StateType::MapPair MapPair;

	const GraphType& targetGraph, & queryGraph;
	vector<NodeIDType> matchSequence;
	StateType mapState;
	AnswerReceiverType& answerReceiver;
	bool induceGraph;
	bool onlyNeedOneSolution;
public:
	SubgraphIsomorphismThread() = default;
	SubgraphIsomorphismThread(const GraphType& _queryGraph, const GraphType& _targetGraph, AnswerReceiverType& _answerReceiver, bool _induceGraph = true,
		bool _onlyNeedOneSolution = true, vector<NodeIDType>& _matchSequence = vector<NodeIDType>())
		:queryGraph(_queryGraph), targetGraph(_targetGraph), answerReceiver(_answerReceiver), induceGraph(_induceGraph), onlyNeedOneSolution(_onlyNeedOneSolution), mapState(_queryGraph, _targetGraph) {
		if (_matchSequence.size() == 0) {
			auto t1 = clock();
			if (matchSequence.size() == 0) {
				if (typeid(_MatchOrderSelector) != typeid(void))matchSequence = _MatchOrderSelector::run(_queryGraph, _targetGraph);
				else matchSequence = MatchOrderSelectorVF3<GraphType>::run(_queryGraph, _targetGraph);
			}
			TIME_COST_PRINT("match order selete time : ", clock() - t1);
			TRAVERSE_SET(nodeID, matchSequence) cout << nodeID << " ";
			cout << endl;
		}
		else matchSequence = _matchSequence;
	}
	void run() {
		return;
	}

};

}