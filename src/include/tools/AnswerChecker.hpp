#include "graph/Graph.hpp"
#include "si/si_marcos.h"
#include <cstring>
#include <vector>
#include <iostream>
#include <memory>
#include <time.h>
#include <map>
#include <fstream>
#include <typeinfo>
#include <unordered_map>

using namespace std;
using namespace wg;
template<class EdgeLabel>
class  AnswerChecker {
	using GraphType = GraphS<EdgeLabel>;
	typedef vector<NodeIDType> SolutionType;
	typedef vector<SolutionType> SolutionsType;

	bool normalCheck(const GraphType &query, const GraphType &target, const SolutionType& solution) {
		int n = query.Size();
		for (int i = 0; i < n; ++i){ 
			const auto from = solution[i];
			const auto outEdges = query.GetOutEdges(from);
			for (const auto &edge:outEdges){
				const auto to = solution[edge.target()];
				if(target.ExistEdge(from, to, edge.label()) == false) return false;
			}
		}
		return true;
	}
	bool induceCheck(const GraphType &query, const GraphType &target, const SolutionType& solution) {
		if (normalCheck(query,target,solution) == false)return false;
		int m = target.Size();
		//NodeIDType rMapping[m];
		vector<NodeIDType> rMapping(m);
		for (int i = 0; i < m; ++i){ 
			rMapping[i] = NO_MAP;
		}
		for (int i = 0; i < solution.size(); ++i){ 
			rMapping[solution[i]] = i;
		}

		for (int  i= 0;  i < m; i++){ 
			const auto from = rMapping[i];
			if(from == NO_MAP) continue;
			for (const auto& edge : target.GetOutEdges(from)) {
				const auto to = rMapping[edge.target()];
				if (to == NO_MAP)continue;
				if (query.ExistEdge(from, to, edge.label()) == false)return false;
			}
		}
		return true;
	}
public:
	enum check_type { INDUCE, NORMAL };
	void run(const GraphType &query, const GraphType &target,const SolutionsType&solutions, check_type ct) {
		size_t solutionCount = 0;
		for (const auto& oneS : solutions) {
			cout << ++solutionCount << " ";
			bool is;
			if (ct == check_type::NORMAL) is = normalCheck(query,target,oneS);
			else if (ct == check_type::INDUCE) is = induceCheck(query,target,oneS);
			else {
				cout << "error occur" << endl;
				return;
			}
			if (!is) cout << "false" << endl;
			else cout << "true" << endl;
		}
	}
};
