#pragma once
#include<unordered_set>
#include<vector>
#include<set>
#include<map>
#include"graph/Graph.hpp"
#include"si/ThreadRelatedClass.hpp"
#include<utility>
#include<numeric>
using namespace std;
namespace wg {

template<class T>
double mean(const T begin, const T end) {
	if (begin == end)return 0;
	return (std::accumulate(begin, end, 0.0)) / (end - begin);
}
template<class T>
double variance(const T begin, const T end, double mean) {
	size_t denominator = end - begin;
	if (denominator <= 1)return 0;
	double answer = 0;
	LOOP(it, begin, end) answer += pow(*it - mean, 2);
	return answer / (denominator - 1);
}
template<class T>
double variance(const T begin, const T end) {
	return variance(begin, end, mean(begin, end));
}


template<class _GraphType>
class MatchOrderSelector {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	static vector<NodeIDType> run(const GraphType& graph, const GraphType& targetGraph) {
		typedef set<NodeIDType> Set;
		Set nodeNotInMatchSet;
		for (const auto node : graph.nodes()) {
			nodeNotInMatchSet.insert(node.id());
		}
		Set ioSet;

		typedef const NodeType* NodeCPointer;
		typedef pair<NodeCPointer, double> NodeMatchPointPair;
		vector<int>  ioMap;

		ioMap.resize(graph.size() + 1);

		const auto calNodeMatchPoint = [&](const NodeType& node) {
			double p1 = node.inEdgesNum() + node.outEdgesNum() + ioMap[node.id()] * 2;
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set& s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {
				const auto& tempNode = graph.node(tempNodeID);
				const auto tempPoint = calNodeMatchPoint(tempNode);
				if (tempPoint > nodePoint) {
					answer = &tempNode;
					nodePoint = tempPoint;
				}
			}
			return NodeMatchPointPair(answer, nodePoint);
		};

		vector<NodeIDType> matchSequence;
		matchSequence.reserve(graph.size() + 1);

		while (nodeNotInMatchSet.empty() == false) {
			NodeMatchPointPair nodePointPair;
			if (ioSet.empty() == false) {
				const auto pair = seleteAGoodNodeToMatch(ioSet);
				nodePointPair = pair;
			}
			else nodePointPair = seleteAGoodNodeToMatch(nodeNotInMatchSet);
			assert(nodePointPair.first != nullptr && "error happened");
			const auto& node = *nodePointPair.first;

			const auto nodeID = node.id();
			matchSequence.push_back(nodeID);
			nodeNotInMatchSet.erase(nodeID);
			ioSet.erase(nodeID);

			for (const auto& tempEdge : node.inEdges()) {
				const auto sourceNodeID = tempEdge.source();
				if (IN_SET(nodeNotInMatchSet, sourceNodeID)) {
					ioSet.insert(sourceNodeID);
					ioMap[sourceNodeID]++;

				}
			}
			for (const auto& tempEdge : node.outEdges()) {
				const auto targetNodeID = tempEdge.target();
				if (IN_SET(nodeNotInMatchSet, targetNodeID)) {
					ioSet.insert(targetNodeID);
					ioMap[targetNodeID]++;
				}
			}

		}
		return matchSequence;
	}


};

template<class _GraphType>
class MatchOrderSelectorTest {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	static vector<NodeIDType> run(const GraphType& graph, const GraphType& targetGraph) {
		typedef unordered_set<NodeIDType> Set;
		Set nodeNotInMatchSet;
		nodeNotInMatchSet.reserve(graph->size());
		for (const auto node : graph.nodes()) {
			nodeNotInMatchSet.insert(node.id());
		}
		Set ioSet;
		ioSet.reserve(graph->size());

		typedef const NodeType* NodeCPointer;
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		vector<int>  ioMap;

		ioMap.resize(graph.size() + 1);

		const auto calNodeMatchPoint = [&](const NodeType& node) {
			double p1 = node.inEdgesNum() + node.outEdgesNum() + ioMap[node.id()] * 2;
			for (const auto& edge : node.inEdges()) {
				const auto nodeID = edge.source();
				const auto& tempNode = graph.node(nodeID);
				p1 += tempNode.inEdgesNum() + tempNode.outEdgesNum();
			}
			for (const auto& edge : node.outEdges()) {
				const auto nodeID = edge.target();
				const auto& tempNode = graph.node(nodeID);
				p1 += tempNode.inEdgesNum() + tempNode.outEdgesNum();
			}
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set& s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {

				const auto& tempNode = graph.node(tempNodeID);
				//			if (ioMap[tempNodeID] == tempNode.inEdgesNum() + tempNode.outEdgesNum()) return NodeMatchPointPair(&tempNode, nodePoint);
				const auto tempPoint = calNodeMatchPoint(tempNode);
				if (tempPoint > nodePoint) {
					answer = &tempNode;
					nodePoint = tempPoint;
				}
			}
			return NodeMatchPointPair(answer, nodePoint);
		};

		vector<NodeIDType> matchSequence;
		matchSequence.reserve(graph.size() + 1);

		while (nodeNotInMatchSet.empty() == false) {
			NodeMatchPointPair nodePointPair;
			if (ioSet.empty() == false) {
				const auto pair = seleteAGoodNodeToMatch(ioSet);
				nodePointPair = pair;
			}
			else nodePointPair = seleteAGoodNodeToMatch(nodeNotInMatchSet);
			assert(nodePointPair.first != nullptr && "error happened");
			const auto& node = *nodePointPair.first;

			const auto nodeID = node.id();
			matchSequence.push_back(nodeID);
			nodeNotInMatchSet.erase(nodeID);
			ioSet.erase(nodeID);

			for (const auto& tempEdge : node.inEdges()) {
				const auto sourceNodeID = tempEdge.source();
				if (IN_SET(nodeNotInMatchSet, sourceNodeID)) {
					ioSet.insert(sourceNodeID);
					ioMap[sourceNodeID]++;

				}
			}
			for (const auto& tempEdge : node.outEdges()) {
				const auto targetNodeID = tempEdge.target();
				if (IN_SET(nodeNotInMatchSet, targetNodeID)) {
					ioSet.insert(targetNodeID);
					ioMap[targetNodeID]++;
				}
			}

		}
		return matchSequence;
	}


};
template<class _GraphType>
class MatchOrderSelectorSI {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	static vector<NodeIDType> run(const GraphType& graph, const GraphType& targetGraph) {
		vector<NodeIDType> matchSequence;
		matchSequence.reserve(graph.size() + 1);
		unordered_map<NodeLabelType, size_t> pgLabelNum = graph.labelNum(), tgLabelNum = targetGraph.labelNum();
		unordered_map<NodeLabelType, size_t> pgLabelInMax = graph.labelMaxIn(), tgLabelInMax = targetGraph.labelMaxIn();
		unordered_map<NodeLabelType, size_t> pgLabelOutMax = graph.labelMaxOut(), tgLabelOutMax = targetGraph.labelMaxOut();
		vector< vector< size_t > > tgin, tgout;
		//a simple check;
		{
			if (pgLabelNum.size() > tgLabelNum.size()) {
				cout << "No solution" << endl;
				exit(0);
			}
			for (const auto p : pgLabelNum) {
				auto l = p.first;
				auto num = p.second;
				if (pgLabelInMax[l] > tgLabelInMax[l] || num > tgLabelNum[l] || pgLabelOutMax[l] > tgLabelOutMax[l]) {
					cout << "no solution\n";
					exit(1);
				}
			}
		}
		size_t labelMax = 0;
		for (auto& p : tgLabelNum)labelMax = max(labelMax, p.first);
		tgin.resize(labelMax + 1);
		tgout.resize(labelMax + 1);
		LOOP(i, 0, tgLabelNum.size()) {
			tgin[i].resize(tgLabelInMax[i] + 1);
			tgout[i].resize(tgLabelOutMax[i] + 1);
		}

		for (auto& node : targetGraph.nodes()) {
			tgin[node.label()][node.inEdgesNum()]++;
			tgout[node.label()][node.outEdgesNum()]++;
		}
		LOOP(i, 0, labelMax + 1) {
			size_t nodeCount = tgLabelNum[i];
			int last = 0;
			LOOP(j, 0, tgout[i].size()) {
				nodeCount -= last;
				last = tgout[i][j];
				tgout[i][j] = nodeCount;
			}
			last = 0;
			nodeCount = tgLabelNum[i];
			LOOP(j, 0, tgin[i].size()) {
				nodeCount -= last;
				last = tgin[i][j];
				tgin[i][j] = nodeCount;
			}
		}
		double* possibility = new double[graph.size()];
		pair<NodeIDType, pair<double, size_t>>* sortPoss = new pair<NodeIDType, pair<double, size_t>>[graph.size()];

		unordered_set<NodeIDType> notInSeq;
		for (auto node : graph.nodes()) {
			auto id = node.id();
			notInSeq.insert(id);
			auto label = node.label();
			possibility[id] = (double)(tgin[label][node.inEdgesNum()] * tgout[label][node.outEdgesNum()]) / (targetGraph.size() * targetGraph.size());

			sortPoss[id] = pair<NodeIDType, pair<double, size_t>>(id, pair<double, size_t>(possibility[id], node.outEdgesNum() + node.inEdgesNum()));

		}

		sort(sortPoss, sortPoss + graph.size(), [](const pair<NodeIDType, pair<double, size_t>>& a, const pair<NodeIDType, pair<double, size_t>>& b)
			{
				if (fabs(a.second.first - b.second.first) < 1E-20) {
					return a.second.second > b.second.second;
				}
				return a.second.first < b.second.first;
			});

		size_t maxDegreeInSeq = 0;
		map<NodeIDType, size_t> ioMap;
		auto* sortPossPoint = sortPoss;
		int seqID = -1;
		auto chooseNode = [&]() {
			int nowid = -1;
			double nowposs = 1;
			if (ioMap.empty() == false) {
				maxDegreeInSeq = 0;
				nowid = -1;
				nowposs = 1;
				for (auto it = ioMap.begin(); it != ioMap.end(); ++it) {
					if (it->second > maxDegreeInSeq) {
						nowid = it->first;
						nowposs = possibility[nowid];
						maxDegreeInSeq = it->second;
					}
					else if (it->second == maxDegreeInSeq) {
						if (fabs(nowposs - possibility[it->second]) < 1E-20) {
							auto nownode = graph.node(nowid);
							auto thisnode = graph.node(it->first);
							if (nownode.outEdgesNum() + nownode.inEdgesNum() < thisnode.outEdgesNum() + thisnode.inEdgesNum()) {
								nowid = it->first;
								nowposs = possibility[nowid];
								maxDegreeInSeq = it->second;
							}
						}
						else if (nowposs > possibility[it->second]) {
							nowid = it->first;
							nowposs = possibility[nowid];
							maxDegreeInSeq = it->second;
						}
					}

					else continue;
				}
			}
			else {
				while (NOT_IN_SET(notInSeq, sortPossPoint->first))sortPossPoint++;  //node already in seq;
				nowid = sortPossPoint->first;
				++sortPossPoint;
			}
			return nowid;
		};

		seqID = chooseNode();
		do {
			matchSequence.push_back(seqID);
			notInSeq.erase(seqID);
			ioMap.erase(seqID);
			auto node = graph.node(seqID);
			for (auto& edge : node.inEdges()) {
				auto temp = edge.source();
				if (IN_SET(notInSeq, temp)) ioMap[temp]++;
			}
			for (auto& edge : node.outEdges()) {
				auto temp = edge.target();
				if (IN_SET(notInSeq, temp)) ioMap[temp]++;
			}
			if (notInSeq.empty())break;
			seqID = chooseNode();
		} while (notInSeq.empty() == false);
		delete[]sortPoss;
		delete[]possibility;
		return matchSequence;
	}


};

template<class _GraphType>
class MatchOrderSelectorSI_T {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	static vector<NodeIDType> run(const GraphType& graph, const GraphType& targetGraph) {
		vector<NodeIDType> matchSequence;
		matchSequence.reserve(graph.size() + 1);
		unordered_map<NodeLabelType, size_t> pgLabelNum = graph.labelNum(), tgLabelNum = targetGraph.labelNum();
		unordered_map<NodeLabelType, size_t> pgLabelInMax = graph.labelMaxIn(), tgLabelInMax = targetGraph.labelMaxIn();
		unordered_map<NodeLabelType, size_t> pgLabelOutMax = graph.labelMaxOut(), tgLabelOutMax = targetGraph.labelMaxOut();
		vector<size_t> label_count;
		//a simple check;
		{
			if (pgLabelNum.size() > tgLabelNum.size()) {
				cout << "No solution" << endl;
				exit(0);
			}
			for (const auto p : pgLabelNum) {
				auto l = p.first;
				auto num = p.second;
				if (pgLabelInMax[l] > tgLabelInMax[l] || num > tgLabelNum[l] || pgLabelOutMax[l] > tgLabelOutMax[l]) {
					cout << "no solution\n";
					exit(1);
				}
			}
		}
		size_t labelMax = 0, in_max = 0, out_max=0;
		for (auto& p : tgLabelNum)labelMax = max(labelMax, p.first);
		for (auto& p : tgLabelInMax)in_max = max(in_max, p.second);
		for (auto& p : tgLabelOutMax)out_max = max(out_max, p.second);
		label_count.resize(labelMax + 1);
		TwoDArray<size_t> degree_count(in_max+1, out_max+1);
	
		for (auto& node : targetGraph.nodes()) {
			degree_count[node.inEdgesNum()][node.outEdgesNum()]++;
			label_count[node.label()]++;
		}
		for (long long i = in_max - 1; i >= 0; --i) {
			for (long long j = out_max - 1; j >= 0; --j) {
				degree_count[i][j] += degree_count[i + 1][j] + degree_count[i][j + 1] - degree_count[i + 1][j + 1];
			}
		}
	
		double* possibility = new double[graph.size()];
		pair<NodeIDType, pair<double, size_t>>* sortPoss = new pair<NodeIDType, pair<double, size_t>>[graph.size()];

		unordered_set<NodeIDType> notInSeq;
		for (auto& node : graph.nodes()) {
			auto id = node.id();
			notInSeq.insert(id);
			auto label = node.label();
			possibility[id] = (double)(label_count[label] * degree_count[node.inEdgesNum()][node.outEdgesNum()]) / (targetGraph.size() * targetGraph.size() );

			sortPoss[id] = pair<NodeIDType, pair<double, size_t>>(id, pair<double, size_t>(possibility[id], node.outEdgesNum() + node.inEdgesNum()));

		}

		sort(sortPoss, sortPoss + graph.size(), [](const pair<NodeIDType, pair<double, size_t>>& a, const pair<NodeIDType, pair<double, size_t>>& b)
			{
				if (fabs(a.second.first - b.second.first) < 1E-300) {
					return a.second.second > b.second.second;
				}
				return a.second.first < b.second.first;
			});

		size_t maxDegreeInSeq = 0;
		map<NodeIDType, double> ioMap;
		auto* sortPossPoint = sortPoss;
		int seqID = -1;
		auto chooseNode = [&]() {
			int nowid = -1;
			double nowposs = 2;
			if (ioMap.empty() == false) {
				for (auto it = ioMap.begin(); it != ioMap.end(); ++it) {
					if (fabs(it->second - nowposs) < 1e-300) {
						if (graph[it->first].outEdgesNum() + graph[it->first].inEdgesNum() > graph[nowid].outEdgesNum() + graph[nowid].inEdgesNum()) {
							nowposs = it->second;
							nowid = it->first;
						}
					}
					else if (it->second < nowposs) {
						nowposs = it->second;
						nowid = it->first;
					}
				}
			}
			else {
				while (NOT_IN_SET(notInSeq, sortPossPoint->first))sortPossPoint++;  //node already in seq;
				nowid = sortPossPoint->first;
				++sortPossPoint;
			}
			return nowid;
		};

		seqID = chooseNode();
		do {
			matchSequence.push_back(seqID);
			notInSeq.erase(seqID);
			ioMap.erase(seqID);
			if (notInSeq.empty())break;
			auto& node = graph.node(seqID);
			for (auto& edge : node.inEdges()) {
				auto temp = edge.source();
				if (IN_SET(notInSeq, temp)) {
					if (fabs(ioMap[temp] - 0) < 1e-300)ioMap[temp] = possibility[temp];
					ioMap[temp] *= possibility[seqID];
				};
			}
			for (auto& edge : node.outEdges()) {
				auto temp = edge.target();
				if (IN_SET(notInSeq, temp)) {
					if (fabs(ioMap[temp] - 0) < 1e-300)ioMap[temp] = possibility[temp];
					ioMap[temp] *= possibility[seqID];
				}
			}
			
			seqID = chooseNode();
		} while (true);
		delete[]sortPoss;
		delete[]possibility;
		return matchSequence;
	}


};
}