#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include"VF2.hpp"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
using namespace std;
static long t = 0;

int main(int argc, char * argv[]) {
	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;

	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath;
	bool induceGraph = true, onlyNeedOneSolution = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
	induceGraph = cmdl[{"-induce-graph", "-induce"}];
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];

	const GraphType* queryGraph = LADReader<GraphType>::readGraph(queryGraphPath),
		*targetGraph = LADReader<GraphType>::readGraph(targetGraphPath);

	VF2<StateType> vf2(*targetGraph, *queryGraph, induceGraph, onlyNeedOneSolution);
	//	VF2<GraphType> vf2(targetGraph, queryGraph, false);

	auto t1 = clock();
	vf2.run();
	/*
	int sc = 0;
	for (auto oneSolution : vf2.getAnswer()) {
		cout << "Solution : " << sc++ << endl;
		typedef unordered_map<const NodeType*, const NodeType*>::const_iterator itType;
		for (auto it : oneSolution) {
			cout << '(' <<it.first << "," << it.second<<") " ;
		}
		cout << endl;
	}*/
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	return 0;
}

