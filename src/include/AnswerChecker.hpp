#include"Graph.hpp"
#include"Node.hpp"
#include"Edge.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<map>
#include<fstream>
#include<MatchOrderSelector.hpp>
#include<typeinfo>
#define TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , VF2 will use default MatchOrderSelector.

*/
template<class GraphType,class SolutionsType,class SolutionType>
class  AnswerChecker{
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;

	const GraphType& bigGraph, & smallGraph;
	const SolutionsType solutions;
	bool normalCheck(const SolutionType& solution) {
		for (const auto& node : bigGraph.nodes()) {
			const auto from = node.id();
			for (const auto& edge : node.getOutEdges()) {
				const auto to = edge.getTargetNodeID();
				if (bigGraph.existEdge(solution[from],solution[to], edge.getLabel()) == false) return false;
			}
		}
		return true;
	}
public:
	enum check_type{INDUCE,NORMAL};
	AnswerChecker() = default;
	~AnswerChecker() = default;
	AnswerChecker(const GraphType &_b,const GraphType &_s,const SolutionsType &_solutions):bigGraph(_b),smallGraph(_s),solutions(_solutions){}
	void run(check_type ct) {
		if (ct == check_type::NORMAL) {
			size_t solutionCount = 0;
			for (const auto &oneS : solutions) {
				cout << ++solutionCount << " ";
				if (normalCheck(oneS)) cout << "false" << endl;
				else cout << "true" << endl;

			}
		}
	}
};