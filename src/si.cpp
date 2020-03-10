#include"si/SubgraphIsomorphism.hpp"
#include"tools/GraphReader.hpp"
#include"tools/argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"si/AnswerReceiver.hpp"
#include"si/SubgraphIsomorphismThread.hpp"
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
//	typedef Edge<int> EdgeType;
	typedef EdgeSimple<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;



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
//	PRINT_TIME_COST_S("read graph time : ", clock() - t1);
//	cout << "read graph finish" << endl;

	t1 = clock();

	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

//	PRINT_TIME_COST_S("sort edge time : ",clock()- t1);
	vector<NodeIDType> ms =move(readMatchSequence(matchOrderPath));
//	t1 = clock();
	if (threadNum > 1) {
		AnswerReceiverThread answerReceiver(answerPath);
		SubgraphIsomorphismThread<GraphType, AnswerReceiverThread, MatchOrderSelectorType> si(*queryGraph, *targetGraph, answerReceiver, threadNum, onlyNeedOneSolution, ms);
		si.run();
//		cout << "ok\n";
		answerReceiver.finish();
	}
	else {
		AnswerReceiver answerReceiver(answerPath);
		SubgraphIsomorphism<GraphType, AnswerReceiver, MatchOrderSelectorType> si(*queryGraph, *targetGraph, answerReceiver, onlyNeedOneSolution, ms);
		si.run();
		answerReceiver.finish();
	}
	double TimeC = clock()-t1;
	cout << "time cost : " << (TimeC) / CLOCKS_PER_SEC<<" s  "<< (TimeC) / (CLOCKS_PER_SEC/1000)<<" ms" << endl;
	delete queryGraph;
	delete targetGraph;

	return 0;
}

