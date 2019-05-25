#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
#include<iostream>
using namespace std;
template<typename StateType>
class VF2 {
	typedef typename StateType::GraphType GraphType;
	typedef typename StateType::NodeType NodeType;
	typedef typename StateType::EdgeType EdgeType;
	
	typedef typename StateType::MapType MapType;


	const GraphType &targetGraph, &queryGraph;
	vector<MapType> mappings;
	bool onlyNeedOneSolution = true;
	bool induceGraph = true;
	bool goDeeper(StateType &s)
	{
		if (s.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution(s);
			return true;
		}
		for (const auto &tempCanditatePair : s.calCandidatePairs()) {
			if (s.checkCanditatePairIsAddable(tempCanditatePair)) {
				s.addCanditatePairToMapping(tempCanditatePair);
				if (goDeeper(s) && this->onlyNeedOneSolution) return true;
				s.deleteCanditatePairToMapping(tempCanditatePair);
			}
		}
		return false;
	}
public:
	VF2() = default;
	~VF2() = default;
	VF2(const GraphType &_targetGraph, const GraphType &_queryGraph, bool _induceGraph = true, bool _onlyNeedOneSolution = true)
		:targetGraph(_targetGraph), queryGraph(_queryGraph), onlyNeedOneSolution(_onlyNeedOneSolution), induceGraph(_induceGraph) {};
	void ToDoAfterFindASolution(const StateType &s) {
		mappings.push_back(s.getMap());
//		cout << mappings.size() << endl;
	/*	for (auto it : s.getMap()) {
			assert(it.first != nullptr && it.second != nullptr && "Map exist nullptr");
			cout << '(' << it.first->getID() << "," << it.second->getID() << ')' << endl;
		}*/
/*		cout << "AUX MAPPING" << endl;
		for (auto it : s.getAuxMap()) {
			assert(it.first != nullptr && it.second != nullptr && "AuxMap exist nullptr");
			cout << '(' << it.first->getID() << "," << it.second->getID() << ')' << endl;
		}*/
	}
	void run()
	{
		StateType initialState = StateType(targetGraph, queryGraph, induceGraph);
		if(queryGraph.graphSize()<=targetGraph.graphSize()) goDeeper(initialState);
	}
	vector<MapType> getAnswer()const {
		return mappings;
	}
};