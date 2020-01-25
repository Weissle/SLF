#include"Graph.hpp"
#include"Node.hpp"
#include"Edge.hpp"
#include<vector>
#include<iostream>
#include<memory>
#include<time.h>
#include<map>
#include<fstream>
#include<MatchOrderSelector.hpp>
#include<typeinfo>
#include<unordered_map>
#define TIME_COUNT
using namespace std;
/*
About MatchOrderSelector,if MatchOrderSelector is  void type and you do not specify a match order , VF2 will use default MatchOrderSelector.

*/
template<class GraphType>
class  AnswerChecker{
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	typedef typename GraphType::EdgeType EdgeType;
	typedef typename EdgeType::EdgeLabelType EdgeLabelType;
	typedef vector<int> SolutionType;
	typedef vector<SolutionType> SolutionsType;
	
	const GraphType& bigGraph, & smallGraph;
	const SolutionsType solutions;
	bool normalCheck(const SolutionType& solution) {
		for (const auto& node : smallGraph.nodes()) {
			const auto from = solution[node.id()];
			for (const auto& edge : node.getOutEdges()) {
				const auto to = solution[edge.getTargetNodeID()];
				if (bigGraph.existEdge(from, to, edge.getLabel()) == false) {
					return false;
				}
			}
		}
		return true;
	}
	bool induceCheck(const SolutionType& solution) {
		if (normalCheck(solution) == false)return false;
		unique_ptr<bool[]> inM(new bool[bigGraph.size()]());
		unordered_map<NodeIDType, NodeIDType> rm;
		rm.reserve(smallGraph.size());
		LOOP(i,0,solution.size()) {
			const auto temp = solution[i];
			inM[temp] = true;
			rm[temp] = i;
		}
		for (const auto& node : bigGraph.nodes()) {
			const auto from = node.id();
			if (inM[from] == false)continue;
			for (const auto& edge : node.getOutEdges()) {
				const auto to = edge.getTargetNodeID();
				if (inM[to] == false)continue;
				if (smallGraph.existEdge(rm[from], rm[to], edge.getLabel()) == false)return false;
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
		size_t solutionCount = 0;
		for (const auto &oneS : solutions) {
			cout << ++solutionCount << " ";
			bool is;
			if (ct == check_type::NORMAL) is = normalCheck(oneS);
			else if (ct == check_type::INDUCE) is = induceCheck(oneS);
			else {
				cout << "error occur" << endl;
					return;
			}
			if (!is) cout << "false" << endl;
			else cout << "true" << endl;
		}
	}
};