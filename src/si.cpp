#include"si/SubgraphIsomorphism.hpp"
#include"tools/GraphReader.hpp"
#include"tools/argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include<string>
#include<memory>
#include"si/AnswerReceiver.hpp"
#include"si/MatchOrderSelector.hpp"
#include"si/SubgraphIsomorphismThread.hpp"
using namespace std;
static long t = 0;
using namespace wg;

std::vector<size_t> readMatchSequence(std::string&);
int main(int argc, char* argv[]) {
	typedef EdgeSimple<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;



	argh::parser cmdl({ "self-order","-so","-thread","-t","-limits","-l","-print-solution"});
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath, matchOrderPath;
	size_t threadNum = 0;
	bool induceGraph = true, matchOrder = false;
	size_t limits = 0;
	queryGraphPath = cmdl[1];
	targetGraphPath = cmdl[2];
	//	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	cmdl({ "-thread","-t" }) >> threadNum;
	cmdl({ "-self-order","-so" }) >> matchOrderPath;
	cmdl({ "-limits","-l" }) >> limits;
	bool print_solution = cmdl["-print-solution"];
	//match order
#define MOS_SI_TEST
#if defined(MOS_SI)
	typedef MatchOrderSelectorSI<GraphType> MatchOrderSelectorType;
#elif defined(MOS_SI_TEST)
	typedef MatchOrderSelectorSI_T<GraphType> MatchOrderSelectorType;
#endif
	// graph type ( store in files ) and read graph
#define GRF_L
#ifdef GRF_L
	typedef GRFGraphLabel<GraphType, size_t> GraphReader;
#elif defined(LAD)
	typedef LADReader<GraphType> GraphReader;
#elif defined ARG_NL
	typedef ARGGraphNoLabel<GraphType> GraphReader;
#endif
	IndexTurner<size_t> turner;
	time_t t1 = time(0);
	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath, turner),
		* targetGraph = GraphReader::readGraph(targetGraphPath, turner);

	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();
	vector<NodeIDType> ms;
	if (matchOrderPath.size())  ms = move(readMatchSequence(matchOrderPath));
	else ms = MatchOrderSelectorType::run(*queryGraph, *targetGraph);
	shared_ptr<const vector<NodeIDType>> ms_ptr = make_shared<const vector<NodeIDType>>(ms);
	size_t solutions = 0;
	size_t call_times = 0;

	if (threadNum > 1) {
		AnswerReceiverThread answerReceiver(print_solution);
		SubgraphIsomorphismThread<GraphType, AnswerReceiverThread> si(*queryGraph, *targetGraph, answerReceiver, threadNum, limits, ms_ptr);
		si.run();
		solutions = answerReceiver.solutionsCount();
	}
	else {
		AnswerReceiver answerReceiver(print_solution);
		SubgraphIsomorphism<GraphType, AnswerReceiver> si(*queryGraph, *targetGraph, answerReceiver, limits, ms_ptr);
		si.run();
		solutions = answerReceiver.solutionsCount();
		call_times = si.callTimes();
	}
	time_t TimeC = time(0) - t1;
	delete queryGraph;
	delete targetGraph;
	std::cout << "[" + std::string(queryGraphPath) + "," + std::string(targetGraphPath) + "," + std::to_string(solutions) + +"," + std::to_string(TimeC) + "]" << endl;
	if (call_times)	std::cout << "[" + std::string(queryGraphPath) + "," + std::string(targetGraphPath) + "," + std::to_string(solutions) + +"," + std::to_string(call_times) + "]" << endl;


	return 0;

}

std::vector<size_t> readMatchSequence(std::string& matchOrderPath) {
	std::vector<size_t> ms;
	if (matchOrderPath.empty() == false) {
		std::fstream f;
		f.open(matchOrderPath.c_str(), std::ios_base::in);
		if (f.is_open() == false) {
			std::cout << matchOrderPath << " open fail" << std::endl;
			exit(1);
		}
		while (f.eof() == false) {
			size_t temp = UINT32_MAX;
			f >> temp;
			if (temp != UINT32_MAX)ms.push_back(temp);
		}
		f.close();
	}
	return move(ms);
}