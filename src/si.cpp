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
#include"si/ParallelSubgraphIsomorphism.hpp"
using namespace std;
static long t = 0;
using namespace wg;

typedef int EdgeLabelType;
typedef Node<EdgeLabelType> NodeType;
typedef Graph<NodeType, EdgeLabelType> GraphType;

std::vector<size_t> readMatchSequence(std::string&);
shared_ptr<const vector<NodeIDType>> ChooseMatchSequence(GraphType*, GraphType*, std::string&);
int main(int argc, char* argv[]) {
	
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
	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath, turner),
		* targetGraph = GraphReader::readGraph(targetGraphPath, turner);

	time_t t1 = time(0);
	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

	shared_ptr<const vector<NodeIDType>> ms = ChooseMatchSequence(queryGraph, targetGraph, matchOrderPath);
	size_t solutions = 0;
	size_t call_times = 0;

	if (threadNum > 1) {
		AnswerReceiverThread answerReceiver(print_solution);
		ParallelSubgraphIsomorphism<GraphType, AnswerReceiverThread> si(*queryGraph, *targetGraph, answerReceiver, threadNum, limits, ms);
		si.run();
		solutions = answerReceiver.solutionsCount();
	}
	else {
		AnswerReceiver answerReceiver(print_solution);
		SequentialSubgraphIsomorphism<GraphType, AnswerReceiver> si(*queryGraph, *targetGraph, answerReceiver, limits, ms);
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

shared_ptr<const vector<NodeIDType>> ChooseMatchSequence(GraphType* query, GraphType* target, std::string& matchOrderPath) {
//match order
#define MOS_SI
#if defined(MOS_VF3)
	typedef MatchOrderSelectorVF3<GraphType> MatchOrderSelectorType;
#elif defined(MOS_SI)
	typedef MatchOrderSelectorSI<GraphType> MatchOrderSelectorType;
#endif
	vector<NodeIDType> ms;
	//read ms from file
	if (matchOrderPath.size()) {
		ms = move(readMatchSequence(matchOrderPath));
	}
	//choose match sequence according to query and target graphs.
	else {
		ms = MatchOrderSelectorType::run(*query, *target);
	}
	return  make_shared<const vector<NodeIDType>>(ms);
}
