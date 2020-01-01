#include"State.hpp"
#include"VF2.hpp"
#include"GraphReader.hpp"
#include"argh.h"
#include<time.h>
#include<iostream>
#include<fstream>
#include"AnswerReceiver.hpp"
using namespace std;
static long t = 0;

int main(int argc, char * argv[]) {
	typedef size_t NodeIDType;
	typedef EdgeVF2<int> EdgeType;
	typedef NodeVF2<EdgeType> NodeType;
	typedef GraphVF2<NodeType, EdgeType> GraphType;
	typedef StateVF2<GraphType> StateType;
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

    typedef VF2<StateType, AnswerReceiverType,MatchOrderSelectorType> VF2Type;

	GraphType* queryGraph = GraphReader::readGraph(queryGraphPath),
		        *targetGraph = GraphReader::readGraph(targetGraphPath);

	cout << "read graph finish" << endl;
	auto t1 = clock();
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
        vf2 = new VF2Type(*targetGraph, *queryGraph, answerReceiver,ms, induceGraph, onlyNeedOneSolution);

    }
    else{
        vf2 =new  VF2Type(*targetGraph, *queryGraph, answerReceiver, induceGraph, onlyNeedOneSolution);
    }


	t1 = clock();
	vf2->run();

	auto t2 = clock();
	cout << "time cost : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	delete queryGraph;
	delete targetGraph;
	delete vf2;
	return 0;
}

