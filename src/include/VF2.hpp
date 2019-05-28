#include"Edge.hpp"
#include"Graph.hpp"
#include"Node.hpp"
#include"State.hpp"
#include<vector>
#include<iostream>
#include<time.h>
#include<map>
#include<fstream>
#define TIME_COUNT
using namespace std;
template<typename StateType>
class VF2 {
	typedef typename StateType::GraphType GraphType;
	typedef typename StateType::NodeType NodeType;
	typedef typename StateType::EdgeType EdgeType;
	
	typedef typename StateType::MapType MapType;
	typedef typename StateType::MapPair MapPair;
	map<int, int> midGraph;
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
		++hitTime;
	//	if((int)hitTime % (int)1E4 == 0)cout << hitTime << endl;
		auto t1 = clock();
		const auto canditarePairs = s.calCandidatePairs();
		auto t2 = clock();
		bool existAA = false;
	/*	bool isOnrightway = true;
		for (auto it : s.getMap(false)) {
		
			if (midGraph[it.first] != it.second) {
				isOnrightway = false;
				break;
			}
		}
		
		if (isOnrightway &&s.getMap(false).size() >= 5) {
			for (auto it : s.getMap(false)) {
				cout << '(' << it.first << "," << it.second << ')' << " ";
			}
			cout << endl;
//			cout << "error on this : " << it.first << " ," << it.second << endl;
		}
/*
		for (const auto &tempCanditatePair : canditarePairs) {
			const auto queryNodeID = tempCanditatePair.getKey();
			const auto targetNodeID = tempCanditatePair.getValue();
			if (midGraph[queryNodeID] == targetNodeID) existAA = true;

		}
		if(!existAA) {
			bool isOnrightway = true;
			for (auto it : s.getMap()) {
				if (midGraph[it.first] != it.second) {
					isOnrightway = false;
					break;
				}
			}
			if(isOnrightway)
			for (auto it : s.getMap()) {
				//			assert(it.first != nullptr && it.second != nullptr && "Map exist nullptr");
				cout << '(' << it.first << "," << it.second << ')' << " ";
			}
			cout << endl;
		}*/
		cal += t2 - t1;
		for (const auto &tempCanditatePair : canditarePairs){
			const auto queryNodeID = tempCanditatePair.getKey();
			const auto targetNodeID = tempCanditatePair.getValue();
			
			t1 = clock();

			bool suitable = s.checkCanditatePairIsAddable(tempCanditatePair);
		/*	if (midGraph[queryNodeID] == targetNodeID && suitable == false) {
				bool isOnrightway = true;
				for (auto it : s.getMap(false)) {

					if (midGraph[it.first] != it.second) {
						isOnrightway = false;
						break;
					}
				}

				if (isOnrightway ) {
					for (auto it : s.getMap(false)) {
						cout << '(' << it.first << "," << it.second << ')' << " ";
					}
					cout << endl;
					cout << "error on this : " << queryNodeID << " ," << targetNodeID << endl;
					system("pause");
				}
			}*/
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

	size_t cal = 0, check = 0, add = 0, del = 0, hitTime = 0;
public:

	VF2() = default;
	~VF2() = default;
	VF2(const GraphType &_targetGraph, const GraphType &_queryGraph, bool _induceGraph = true, bool _onlyNeedOneSolution = true)
		:targetGraph(_targetGraph), queryGraph(_queryGraph), onlyNeedOneSolution(_onlyNeedOneSolution), induceGraph(_induceGraph) {
		fstream f;
		f.open("D:\\Doc\\Code\\Sub-graph-generator\\build\\Release\\midgraph.graph",ios_base::in);
		for (int i = 0; i < queryGraph.graphSize(); ++i) {
			int a;
			int b;
			f >> a >> b;
			midGraph[b] = a;
		}
	
	};
	void ToDoAfterFindASolution(const StateType &s) {
		mappings.push_back(s.getMap());
		cout << "Solution : " << mappings.size() << endl;
		for (auto it : s.getMap()) {
			cout << '(' << it.first << "," << it.second << ')' << " ";
		}
		cout << endl;

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
		cout << "hit times " << hitTime << endl;
		return mappings;
	}
};