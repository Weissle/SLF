#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
class VF2 {
	Graph targetGraph, queryGraph;
	State mapping;
	bool onlyNeedOneSolution = true;
	bool goDeeper(State &s);
public:
	VF2() = default;
	~VF2() = default;
	VF2(Graph &_targetGraph, Graph &_queryGraph);
	virtual void ToDoAfterFindASolution();
	void run();
};