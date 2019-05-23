#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include"VF2.h"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
using namespace std;
int main(int argc,char * argv[]) {
	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;


	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;

	
	const GraphType* queryGraph = LADReader<GraphType>::readGraph(queryGraphPath), 
			*targetGraph = LADReader<GraphType>::readGraph(targetGraphPath);

/*	vector<NodeType> queryNodes{ NodeType(1),NodeType(2) };
	GraphType queryGraph(queryNodes);
	queryGraph.addEdge(1, 2);
	queryGraph.addEdge(2, 1);
	vector<NodeType> targetNodes{ NodeType(1),NodeType(2),NodeType(3),NodeType(4) };
	GraphType targetGraph(targetNodes);
	targetGraph.addEdge(3, 4);
	targetGraph.addEdge(4, 3);
	targetGraph.addEdge(2, 1);
	targetGraph.addEdge(1, 2);*/
//	targetGraph.addEdge(2, 4);

	VF2<GraphType> vf2(*targetGraph, *queryGraph, true, false);
//	VF2<GraphType> vf2(targetGraph, queryGraph, false);

	auto t1 = clock();
	vf2.run();
	int sc = 0;
	for (auto oneSolution : vf2.getAnswer()) {
		cout << "Solution : " << sc++ << endl;
		typedef unordered_map<const NodeType*, const NodeType*>::const_iterator itType;
		for (auto it : oneSolution) {
			cout << '(' <<it.first->getID() << "," << it.second->getID()<<')' << endl;
	//		cout << it.getKey() << " " << it.getValue() << endl;
		}
	}
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
//	system("pause");

	return 0;
}