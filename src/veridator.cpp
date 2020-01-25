#include"include/State.hpp"
#include"include/VF2.hpp"
#include"include/GraphReader.hpp"
#include"include/argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"include/AnswerReceiver.hpp"
#include"AnswerChecker.hpp"
#include"SolutionReader.hpp"
using namespace std;

static long t = 0;

int main(int argc, char* argv[]) {
	typedef size_t NodeIDType;
	typedef EdgeVF2<int> EdgeType;
	typedef NodeVF2<EdgeType> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;
	typedef AnswerReceiver<NodeIDType> AnswerReceiverType;


	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-solution","-induce","-qg" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath, solutionPath;
	bool induceGraph = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
	cmdl({ "-solution" }) >> solutionPath;
	induceGraph = (cmdl[{"-induce"}]) ? false : true;

#define GRF_L
#ifdef GRF_L
	typedef GRFGraphLabel<GraphType> GraphReader;
#elif defined(LAD)
	typedef LADReader<GraphType> GraphReader;
#elif defined ARG_NL
	typedef ARGGraphNoLabel<GraphType> GraphReader;
#endif

	vector< vector<int> > solutions = SolutionReader::readSolutions(solutionPath);


	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		* targetGraph = GraphReader::readGraph(targetGraphPath);
	queryGraph->graphBuildFinish();
	targetGraph->graphBuildFinish();
	typedef AnswerChecker<GraphType> ACer;
	ACer ac(*targetGraph, *queryGraph, solutions);
	ac.run(ACer::check_type::NORMAL);
	cout << endl << endl;
	ac.run(ACer::check_type::INDUCE);
	return 0;
}
