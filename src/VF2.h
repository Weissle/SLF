#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
using namespace std;
template<typename GraphType>
class VF2 {
	typedef typename GraphType::NodeType NodeType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename StateVF2<NodeType, EdgeType> StateType;
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
	virtual void ToDoAfterFindASolution(const StateType &s) {
		mappings.push_back(s.getMap());
	}
	void run()
	{
		//StateVF2<NodeType, EdgeType>
		StateVF2<NodeType, EdgeType> initialState = StateVF2<NodeType, EdgeType>(targetGraph, queryGraph, induceGraph);
		//StateType initialState(   //(targetGraph, queryGraph);
//		StateVF2<NodeType, EdgeType> initialState(const Graph<NodeType, EdgeType> (), queryGraph);
		goDeeper(initialState);
	}
	vector<MapType> getAnswer()const {
		return mappings;
	}
};