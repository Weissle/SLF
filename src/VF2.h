#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
//template<typename NodeIDType, typename EdgeLabelType>
template<typename GraphType,typename StateType>
class VF2 {
//	typedef Graph<NodeIDType, EdgeLabelType> GraphType;
//	typedef State<NodeIDType> StateType;
	GraphType targetGraph, queryGraph;
	StateType mapping;
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
	VF2(GraphType &_targetGraph, GraphType &_queryGraph) :targetGraph(_targetGraph), queryGraph(_queryGraph) {};
	virtual void ToDoAfterFindASolution() {
		return;
	}
	void run()
	{
		State initialState = StateVF2(targetGraph, queryGraph);
		goDeeper(initialState);
	}
};