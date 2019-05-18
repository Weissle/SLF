#include "VF2.h"
#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include"Pair.hpp"

VF2::VF2(Graph & _targetGraph, Graph & _queryGraph):targetGraph(_targetGraph),queryGraph(_queryGraph)
{

}

void VF2::ToDoAfterFindASolution()
{
	return;
}

void VF2::run()
{
	State initialState = StateVF2(targetGraph, queryGraph);
	goDeeper(initialState);
}

bool VF2::goDeeper(State & s)
{
	if (s.isCoverQueryGraph()) {
		if(this->onlyNeedOneSolution ==false) this->ToDoAfterFindASolution();
		return true;
	}
	for (auto tempCanditatePair : s.calCandidatePairs()) {
		if (s.addCanditatePairToMapping(tempCanditatePair)) {
			if (goDeeper(s) && this->onlyNeedOneSolution) return true;
			s.deleteCanditatePairToMapping(tempCanditatePair);
		}
	}
	return false;
}