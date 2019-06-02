#include"State.hpp"
#include"VF2.hpp"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
#include"AnswerReceiver.hpp"
using namespace std;
static long t = 0;

int main(int argc, char * argv[]) {
	typedef int NodeIDType;
	typedef EdgeVF2<NodeIDType, int> EdgeType;
	typedef NodeVF2<int, EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;
	typedef AnswerReceiver<NodeIDType> AnswerReceiverType;
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

	AnswerReceiverType answerReceiver;
	VF2<StateType, AnswerReceiverType> vf2(*targetGraph, *queryGraph, answerReceiver, induceGraph, onlyNeedOneSolution);

	auto t1 = clock();
	vf2.run();

	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;
	return 0;
}

