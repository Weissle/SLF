#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include"VF2.h"
#include<iostream>
using namespace std;
int main() {
	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;

	vector<NodeType> queryNodes{ NodeType(1),NodeType(2) };
	GraphType queryGraph(queryNodes);
	queryGraph.addEdge(1, 2);

	vector<NodeType> targetNodes{ NodeType(1),NodeType(2),NodeType(3),NodeType(4) };
	GraphType targetGraph(targetNodes);
	targetGraph.addEdge(3, 4);

	VF2<GraphType> vf2(targetGraph, queryGraph);
	vf2.run();
	int sc = 0;
	for (auto oneSolution : vf2.getAnswer()) {
		cout << "Solution : " << sc << endl;
		for (auto it : oneSolution) {
			cout << it.getKey() << " " << it.getValue() << endl;
		}
	}



	return 0;
}