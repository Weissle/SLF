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
//	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];

#define MOS_TEST
#ifdef MOS_TEST
	typedef MatchOrderSelectorTest<GraphType> MatchOrderSelectorType;
#elif
	typedef MatchOrderSelector<GraphType> MatchOrderSelectorType;
#endif

#define GRF_L
#ifdef GRF_L
	typedef GRFGraphLabel<GraphType> GraphReader;
#elif defined(LAD)
	typedef LADReader<GraphType> GraphReader;
#elif defined ARG_NL
	typedef ARGGraphNoLabel<GraphType> GraphReader;
#endif

	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		*targetGraph = GraphReader::readGraph(targetGraphPath);

	cout << "read graph finish" << endl;
	auto t1 = clock();
	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

	TIME_COST_PRINT("sort edge time : ",clock()- t1);
	AnswerReceiverType answerReceiver;
	auto ms = MatchOrderSelectorType::run(*queryGraph);
/*	for (auto &id : ms) {
		cout << id << " ";
	}*/
//	VF2<StateType, AnswerReceiverType> vf2(*targetGraph, *queryGraph, answerReceiver,ms, induceGraph, onlyNeedOneSolution);
	VF2<StateType, AnswerReceiverType,MatchOrderSelectorType> vf2(*targetGraph, *queryGraph, answerReceiver, induceGraph, onlyNeedOneSolution);

	t1 = clock();
	vf2.run();

	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;
	return 0;
}

