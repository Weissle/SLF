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
	typedef size_t NodeIDType;
	typedef EdgeVF2<int> EdgeType;
	typedef NodeVF2<EdgeType, int> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;
	typedef AnswerReceiver<NodeIDType> AnswerReceiverType;
	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath;
	bool induceGraph = true, onlyNeedOneSolution = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];

	typedef GRFGraphLabel<GraphType> GraphReader;
//	typedef LADReader<GraphType> GraphReader;

	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		*targetGraph = GraphReader::readGraph(targetGraphPath);

	cout << "read graph finish" << endl;
	auto t1 = clock();
	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

	TIME_COST_PRINT("sort edge time : ",clock()- t1);
	AnswerReceiverType answerReceiver;
	VF2<StateType, AnswerReceiverType> vf2(*targetGraph, *queryGraph, answerReceiver, induceGraph, onlyNeedOneSolution);

	t1 = clock();
	vf2.run();

	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;
	return 0;
}

