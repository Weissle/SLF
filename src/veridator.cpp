#include"tools/GraphReader.hpp"
#include"tools/argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"tools/AnswerChecker.hpp"
#include"tools/SolutionReader.hpp"
#include"tools/IndexTurner.hpp"
using namespace std;
using namespace wg;
static long t = 0;

int main(int argc, char* argv[]) {
	typedef int EdgeLabelType;
	typedef Node<EdgeLabelType> NodeType;
	typedef Graph<NodeType, EdgeLabelType> GraphType;


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

	vector< vector<size_t> > solutions = SolutionReader::readSolutions(solutionPath);

	IndexTurner<size_t> turner;
	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath,turner),
		* targetGraph = GraphReader::readGraph(targetGraphPath,turner);
	queryGraph->graphBuildFinish();
	targetGraph->graphBuildFinish();
	typedef AnswerChecker<GraphType> ACer;
	ACer ac(*targetGraph, *queryGraph, solutions);
	ac.run(ACer::check_type::NORMAL);
	cout << endl << endl;
	ac.run(ACer::check_type::INDUCE);
	return 0;
}
