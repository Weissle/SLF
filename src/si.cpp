#include"State.hpp"
#include"SubgraphIosmorphism.hpp"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"AnswerReceiver.hpp"
#include"SubgraphIosmorphismThread.hpp"
using namespace std;
static long t = 0;
using namespace wg;
int main(int argc, char * argv[]) {
	typedef size_t NodeIDType;
	typedef Edge<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;
	typedef State<GraphType> StateType;



	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg","self-order","-so","-thread","-t" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath,matchOrderPath;
	size_t threadNum = 0;
	bool induceGraph = true, onlyNeedOneSolution = false,matchOrder = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
//	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];
	cmdl({ "-thread","-t" }) >> threadNum;
    cmdl({"-self-order","-so"})>>matchOrderPath;

	//match order
    matchOrder = !matchOrderPath.empty();
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

	vector<NodeIDType> ms;

    if(matchOrder){
        fstream f;
        f.open(matchOrderPath.c_str(),ios_base::in);
        if(f.is_open()==false){
            cout<<matchOrderPath<< " open fail"<<endl;
            exit(1);
        }
		ms.resize(queryGraph->size());
        for (auto i=0;i<ms.size();++i) {
            f>>ms[i];
        }
        f.close();
    }
	t1 = clock();
	if (threadNum > 1) {
		typedef AnswerReceiverThread<NodeIDType> AnswerReceiverType;
		typedef SubgraphIsomorphismThread<StateType, AnswerReceiverType, MatchOrderSelectorType> SIType;
		AnswerReceiverType answerReceiver;
		SIType si(*queryGraph, *targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution, ms);
		si.run();
		answerReceiver.finish();
	}
	else {
		typedef AnswerReceiver<NodeIDType> AnswerReceiverType;
		typedef SubgraphIsomorphism<StateType, AnswerReceiverType, MatchOrderSelectorType> SIType;
		AnswerReceiverType answerReceiver;
		SIType si(*queryGraph, *targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution, ms);
		si.run();
		answerReceiver.finish();
	}
	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;

	return 0;
}

