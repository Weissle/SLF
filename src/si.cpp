#include"State.hpp"
#include"SubgraphIosmorphism.hpp"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"AnswerReceiver.hpp"
using namespace std;
static long t = 0;
using namespace wg;
int main(int argc, char * argv[]) {
	typedef size_t NodeIDType;
	typedef Edge<int> EdgeType;
	typedef Node<EdgeType> NodeType;
	typedef Graph<NodeType, EdgeType> GraphType;
	typedef State<GraphType> StateType;
	typedef AnswerReceiver<NodeIDType> AnswerReceiverType;


	argh::parser cmdl({ "-target-graph","-tg","-query-graph","-qg","self-order","-so" });
	cmdl.parse(argc, argv);
	string queryGraphPath, targetGraphPath,matchOrderPath;
	bool induceGraph = true, onlyNeedOneSolution = false,matchOrder = false;
	cmdl({ "-target-graph","-tg" }) >> targetGraphPath;
	cmdl({ "-query-graph","-qg" }) >> queryGraphPath;
//	induceGraph = (cmdl[{"-no-induce"}]) ? false : true;
	onlyNeedOneSolution = cmdl[{"-one-solution", "-one"}];

    cmdl({"-self-order","-so"})>>matchOrderPath;
    matchOrder = !matchOrderPath.empty();
#define MOS_VF3
#ifdef MOS_TEST
    typedef MatchOrderSelectorTest<GraphType> MatchOrderSelectorType;
#elif defined(MOS_NORMAL)
	typedef MatchOrderSelector<GraphType> MatchOrderSelectorType;
#elif defined(MOS_VF3)
	typedef MatchOrderSelectorVF3<GraphType> MatchOrderSelectorType;
#endif

#define GRF_L
#ifdef GRF_L
	typedef GRFGraphLabel<GraphType> GraphReader;
#elif defined(LAD)
	typedef LADReader<GraphType> GraphReader;
#elif defined ARG_NL
	typedef ARGGraphNoLabel<GraphType> GraphReader;
#endif

    typedef SubgraphIsomorphism<StateType, AnswerReceiverType,MatchOrderSelectorType> VF2Type;
	auto t1 = clock();

	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		        *targetGraph = GraphReader::readGraph(targetGraphPath);
	TIME_COST_PRINT("read graph time : ", clock() - t1);
	cout << "read graph finish" << endl;

	t1 = clock();

	targetGraph->graphBuildFinish();
	queryGraph->graphBuildFinish();

	TIME_COST_PRINT("sort edge time : ",clock()- t1);
	AnswerReceiverType answerReceiver;

	vector<NodeIDType> ms;
	ms.resize(queryGraph->size());
    VF2Type *vf2=nullptr;
    if(matchOrder){
        fstream f;
        f.open(matchOrderPath.c_str(),ios_base::in);
        if(f.is_open()==false){
            cout<<matchOrderPath<< " open fail"<<endl;
            exit(1);
        }
        for (auto i=0;i<ms.size();++i) {
            f>>ms[i];
        }
        f.close();
        vf2 = new VF2Type(*queryGraph, *targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution,ms);

    }
    else{
        vf2 =new  VF2Type(*queryGraph, *targetGraph, answerReceiver, induceGraph, onlyNeedOneSolution);
    }

	t1 = clock();
	vf2->run();

	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete vf2;
	delete queryGraph;
	delete targetGraph;
	
	answerReceiver.finish();
	return 0;
}

