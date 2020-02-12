#include"si/SubgraphIosmorphism.hpp"
#include"tools/GraphReader.hpp"
#include"tools/argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"si/AnswerReceiver.hpp"
#include"si/SubgraphIosmorphismThread.hpp"
using namespace std;
static long t = 0;
using namespace wg;
vector<size_t> readMatchSequence(string &matchOrderPath) {
	vector<size_t> ms;
	if (matchOrderPath.empty()==false) {
		fstream f;
		f.open(matchOrderPath.c_str(), ios_base::in);
		if (f.is_open() == false) {
			cout << matchOrderPath << " open fail" << endl;
			exit(1);
		}
		while (f.eof() == false) {
			size_t temp;
			f >> temp;
			ms.push_back(temp);
		}
		f.close();
	}
	return move(ms);
}
int main(int argc, char * argv[]) {
	typedef size_t NodeIDType;
	typedef Edge<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;
//	typedef State<GraphType> StateType;



	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg","self-order","-so","-thread","-t","-o" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath,matchOrderPath,answerPath="";
	size_t threadNum = 0;
	bool induceGraph = true, onlyNeedOneSolution = false,matchOrder = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
	cmdl({ "-o" }) >> answerPath;
//	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];
	cmdl({ "-thread","-t" }) >> threadNum;
    cmdl({"-self-order","-so"})>>matchOrderPath;

	//match order
#define MOS_VF3
#ifdef MOS_TEST
    typedef MatchOrderSelectorTest<GraphType> MatchOrderSelectorType;
#elif defined(MOS_NORMAL)
	typedef MatchOrderSelector<GraphType> MatchOrderSelectorType;
#elif defined(MOS_VF3)
	typedef MatchOrderSelectorVF3<GraphType> MatchOrderSelectorType;
#endif
	// graph type ( store in files ) and read graph
#define GRF_L
#ifdef GRF_L
	typedef GRFGraphLabel<GraphType> GraphReader;
#elif defined(LAD)
	typedef LADReader<GraphType> GraphReader;
#elif defined ARG_NL
	typedef ARGGraphNoLabel<GraphType> GraphReader;
#endif
	auto t1 = clock();

	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		        *targetGraph = GraphReader::readGraph(targetGraphPath);
	TIME_COST_PRINT("read graph time : ", clock() - t1);
	cout << "read graph finish" << endl;

	t1 = clock();

	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

	TIME_COST_PRINT("sort edge time : ",clock()- t1);
	vector<NodeIDType> ms =move(readMatchSequence(matchOrderPath));
	SubgraphIsomorphism<GraphType> *si;
	AnswerReceiver *answerReceiver;
	t1 = clock();
	if (threadNum > 1) {
		auto answerReceiverThreadPointer = new AnswerReceiverThread(answerPath);
		answerReceiver = answerReceiverThreadPointer;
		si = new SubgraphIsomorphismThread<GraphType, AnswerReceiverThread, MatchOrderSelectorType>(*queryGraph, *targetGraph, *answerReceiverThreadPointer, threadNum, induceGraph, onlyNeedOneSolution, ms);
	}
	else {
		answerReceiver=new AnswerReceiver(answerPath);
		si =new SubgraphIsomorphism_One<GraphType,AnswerReceiver,MatchOrderSelectorType>(*queryGraph, *targetGraph, *answerReceiver, induceGraph, onlyNeedOneSolution, ms);
	}
	si->run();
	answerReceiver->finish();
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;

	return 0;
}

