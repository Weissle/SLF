#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
using namespace std;
template<typename GraphType, typename StateType>
class VF2 {
	//	typedef Graph<NodeIDType, EdgeLabelType> GraphType;
	//	typedef State<NodeIDType> StateType;
	GraphType targetGraph, queryGraph;
	vector<StateType> mappings;
	bool onlyNeedOneSolution = true;
	bool goDeeper(StateType &s)
	{
		if (s.isCoverQueryGraph()) {
			if (this->onlyNeedOneSolution == false) this->ToDoAfterFindASolution();
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
	VF2(GraphType &_targetGraph, GraphType &_queryGraph, bool _onlyNeedOneSolution = true)
		:targetGraph(_targetGraph), queryGraph(_queryGraph), onlyNeedOneSolution(_onlyNeedOneSolution) {};
	virtual void ToDoAfterFindASolution(StateType s) {
		mappings.push_back(s);
	}
	void run()
	{
		State initialState = StateVF2(targetGraph, queryGraph);
		goDeeper(initialState);
	}
	vector<StateType> getAnswer() {
		return mappings;
	}
};