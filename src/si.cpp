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
static long t = 0;/*
bool ASSF(string qp, string tg) {

	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;
	const GraphType* queryGraph = LADReader<GraphType>::readGraph(qp),
		*targetGraph = LADReader<GraphType>::readGraph(tg);
//	cout << queryGraph->checkAddressRight() << " " << queryGraph->checkAddressRight() << endl;

	VF2<StateType> vf2(*targetGraph, *queryGraph, true, false);
	//	VF2<GraphType> vf2(targetGraph, queryGraph, false);
	auto t1 = clock();
	vf2.run();
	int sc = 0;
	auto &temp = vf2.getAnswer();
	auto t2 = clock();
	t += (t2 - t1);
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
//	if (temp.size() < 6)return false;
	delete queryGraph;
	delete targetGraph;
	/*for (auto oneSolution : vf2.getAnswer()) {
		cout << "Solution : " << sc++ << endl;
		typedef unordered_map<const NodeType*, const NodeType*>::const_iterator itType;
		for (auto it : oneSolution) {
			cout << '(' << it.first->getID() << "," << it.second->getID() << ')' << endl;
			//		cout << it.getKey() << " " << it.getValue() << endl;
		}
	}

	return true;
}*/
int main(int argc, char * argv[]) {
	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;

	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
	

		const GraphType* queryGraph = LADReader<GraphType>::readGraph(queryGraphPath),
				*targetGraph = LADReader<GraphType>::readGraph(targetGraphPath);

		VF2<StateType> vf2(*targetGraph, *queryGraph, true, false);
	//	VF2<GraphType> vf2(targetGraph, queryGraph, false);

		auto t1 = clock();
		vf2.run();
		int sc = 0;
		for (auto oneSolution : vf2.getAnswer()) {
			cout << "Solution : " << sc++ << endl;
			typedef unordered_map<const NodeType*, const NodeType*>::const_iterator itType;
			for (auto it : oneSolution) {
				cout << '(' <<it.first << "," << it.second<<')' << endl;
		//		cout << it.getKey() << " " << it.getValue() << endl;
			}
		}
		auto t2 = clock();
		cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	//	system("pause");
	
	int count = 0;
/*	while (count < 5 && ASSF(queryGraphPath, targetGraphPath))count++;
	cout << (double)(t) / (5 * CLOCKS_PER_SEC);*/
	return 0;
}

