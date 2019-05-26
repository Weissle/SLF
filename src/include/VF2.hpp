#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#define TIME_COUNT
using namespace std;
template<typename StateType>
class VF2 {
	typedef typename StateType::GraphType GraphType;
	typedef typename StateType::NodeType NodeType;
	typedef typename StateType::EdgeType EdgeType;
	
	typedef typename StateType::MapType MapType;


	const GraphType &targetGraph, &queryGraph;
	vector<MapType> mappings;
	bool onlyNeedOneSolution = true;
	bool induceGraph = true;
	bool goDeeper(StateType &s)
	{
		if (s.isCoverQueryGraph()) {
			this->ToDoAfterFindASolution(s);
			return true;
		}
#ifdef TIME_COUNT
		auto t1 = clock();
		const auto canditarePairs = s.calCandidatePairs();
		auto t2 = clock();
		cal += t2 - t1;
		for (const auto &tempCanditatePair : canditarePairs){
			t1 = clock();
			bool suitable = s.checkCanditatePairIsAddable(tempCanditatePair);
			t2 = clock();
			check += t2 - t1;
			if(suitable){
				t1 = clock();
				s.addCanditatePairToMapping(tempCanditatePair);
				t2 = clock();
				add += t2 - t1;
				if (goDeeper(s) && this->onlyNeedOneSolution) return true;
				t1 = clock();
				s.deleteCanditatePairToMapping(tempCanditatePair);
				t2 = clock();
				del += t2 - t1;
			}
		}
		
#else
		for (const auto &tempCanditatePair : s.calCandidatePairs()) {
			if (s.checkCanditatePairIsAddable(tempCanditatePair)) {
				s.addCanditatePairToMapping(tempCanditatePair);
				if (goDeeper(s) && this->onlyNeedOneSolution) return true;
				s.deleteCanditatePairToMapping(tempCanditatePair);
			}
		}
#endif
		return false;
	}

	size_t cal = 0, check = 0, add = 0, del = 0;
public:

	VF2() = default;
	~VF2() = default;
	VF2(const GraphType &_targetGraph, const GraphType &_queryGraph, bool _induceGraph = true, bool _onlyNeedOneSolution = true)
		:targetGraph(_targetGraph), queryGraph(_queryGraph), onlyNeedOneSolution(_onlyNeedOneSolution), induceGraph(_induceGraph) {};
	void ToDoAfterFindASolution(const StateType &s) {
		mappings.push_back(s.getMap());
//		cout << mappings.size() << endl;
		cout << "Solution : " << mappings.size() << endl;
		for (auto it : s.getMap()) {
//			assert(it.first != nullptr && it.second != nullptr && "Map exist nullptr");
			cout << '(' << it.first << "," << it.second << ')' << " ";
		}
		cout << endl;
/*		cout << "AUX MAPPING" << endl;
		for (auto it : s.getAuxMap()) {
			assert(it.first != nullptr && it.second != nullptr && "AuxMap exist nullptr");
			cout << '(' << it.first->getID() << "," << it.second->getID() << ')' << endl;
		}*/
	}
	void run()
	{
		StateType initialState = StateType(targetGraph, queryGraph, induceGraph);
		if(queryGraph.graphSize()<=targetGraph.graphSize()) goDeeper(initialState);
	}
	vector<MapType> getAnswer()const {
		cout << "cal Canditate Pairs " << double(cal) / CLOCKS_PER_SEC << endl;
		cout << "check Canditate Pairs " << double(check) / CLOCKS_PER_SEC << endl;
		cout << "add Canditate Pairs " << double(add) / CLOCKS_PER_SEC << endl;
		cout << "delete Canditate Pairs " << double(del) / CLOCKS_PER_SEC << endl;
		return mappings;
	}
};