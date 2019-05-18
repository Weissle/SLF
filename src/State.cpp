#include "State.hpp"

StateVF2::StateVF2(const Graph & _t, const Graph & _q):targetGraph(_t),queryGraph(_q)
{
	
}

bool StateVF2::isCoverQueryGraph()
{
	if (queryGraph.graphSize() == mapping.size()) return true;
	return false;
}
