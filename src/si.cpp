#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include "si/AnswerReceiver.hpp"
#include "si/MatchOrderSelector.hpp"
#include "si/ParallelSubgraphIsomorphism.hpp"
#include "si/SubgraphIsomorphism.hpp"
#include "si/si_marcos.h"
#include "tools/GraphReader.hpp"
#include "tools/argh.h"
using namespace std;
static long t = 0;
using namespace wg;

typedef int EdgeLabelType;
typedef GraphS<EdgeLabelType> GraphType;

// Read the match sequence from file. 
std::vector<NodeIDType> readMatchSequence(const std::string&);
// Choose the match sequence. Create from query and target graphs or read from file.
vector<NodeIDType> ChooseMatchSequence(GraphType const*, GraphType const*,const std::string&);

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

	GraphReader<int> graphReader;
	GraphType* queryGraph = graphReader.ReadFromGRF(queryGraphPath),
	 *targetGraph = graphReader.ReadFromGRF(targetGraphPath);

	//GraphType* queryGraph = graphReader.ReadFromBN(queryGraphPath),
		 //*targetGraph = graphReader.ReadFromBN(targetGraphPath);

	auto t1 = chrono::steady_clock::now();

	targetGraph->SortEdge();
	queryGraph->SortEdge();

	vector<NodeIDType> ms = ChooseMatchSequence(queryGraph, targetGraph, matchOrderPath);
	size_t solutions = 0;
	size_t call_times = 0;

	// multi threads
	if (threadNum > 1) {
		AnswerReceiverThread answerReceiver(print_solution);
		ParallelSubgraphIsomorphism<EdgeLabelType> si;
		si.run(*queryGraph, *targetGraph, &answerReceiver, threadNum, limits, ms);
		solutions = answerReceiver.solutionsCount();
	}
	//single thread
	else {
		AnswerReceiver answerReceiver(print_solution);
		SequentialSubgraphIsomorphism<EdgeLabelType> si(*queryGraph, *targetGraph, &answerReceiver, limits, ms);
		si.run();
		solutions = answerReceiver.solutionsCount();
		call_times = si.callTimes();
	}

	chrono::duration<double> costTime = (chrono::steady_clock::now()-t1);
	delete queryGraph;
	delete targetGraph;
	std::cout << "[" + std::string(queryGraphPath) + "," + std::string(targetGraphPath) + "," + std::to_string(solutions) + +"," + to_string(costTime.count()) + "]" << endl;
	if (call_times)	std::cout << "[" + std::string(queryGraphPath) + "," + std::string(targetGraphPath) + "," + std::to_string(solutions) + +"," + std::to_string(call_times) + "]" << endl;


	return 0;

}

std::vector<NodeIDType> readMatchSequence(const std::string& matchOrderPath) {
	std::vector<NodeIDType> ms;
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
			if (temp != UINT32_MAX) ms.push_back(temp);
		}
		f.close();
	}
	return ms;
}

vector<NodeIDType> ChooseMatchSequence(GraphType const * query, GraphType const * target,const std::string& matchOrderPath) {
//match order
	MatchOrderSelector<EdgeLabelType> orderSelector;

	vector<NodeIDType> ms;
	//read ms from file
	if (matchOrderPath.size()) {
		ms = readMatchSequence(matchOrderPath);
	}
	//choose match sequence according to query and target graphs.
	else {
		//ms = MatchOrderSelectorType::run(*query, *target);
		ms = orderSelector.SLF(*query, *target);
	}
	//cout<<"match sequential:";
	//for (const auto &one:ms){
		//cout<<one<<' ';
	//}
	//cout<<endl;
	return ms;
}
