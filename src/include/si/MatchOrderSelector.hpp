#pragma once
#include<unordered_set>
#include<vector>
#include<set>
#include<map>
#include"graph/Graph.hpp"
#include"si/ThreadRelatedClass.hpp"
#include<utility>
#include<cmath>
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
		unordered_map<NodeLabelType, size_t> pgLabelNum = graph.labelNum(), tgLabelNum = graph.labelNum();
		unordered_map<NodeLabelType, size_t> pgLabelInMax = graph.labelMaxIn();
		unordered_map<NodeLabelType, size_t> pgLabelOutMax = graph.labelMaxOut();
		//a simple check;
		{
			if (pgLabelNum.size() > tgLabelNum.size()) {
				cout << "No solution" << endl;
				exit(0);
			}
		}
		vector<TwoDArray<size_t>> degree_count_label(max(graph.maxLabel(), targetGraph.maxLabel()) + 1);
		for (auto i = 0; i < degree_count_label.size(); ++i) {
			degree_count_label[i] = TwoDArray<size_t>(pgLabelInMax[i] + 1, pgLabelOutMax[i] + 1);
		}
		for (auto& node : targetGraph.nodes()) {
			auto label = node.label();
			auto row = min(node.inEdgesNum(), degree_count_label[label].row() - 1);
			auto col = min(node.outEdgesNum(), degree_count_label[label].column() - 1);
			degree_count_label[label][row][col]++;
		}
		for (auto z = 0; z < degree_count_label.size(); ++z) {
			size_t row_max = degree_count_label[z].row() - 1, col_max = degree_count_label[z].column() - 1;
			for (long long i = degree_count_label[z].row() - 1; i >= 0; --i) {
				for (long long j = degree_count_label[z].column() - 1; j >= 0; --j) {
					auto top = ((i + 1) > row_max) ? 0 : degree_count_label[z][i + 1][j];
					auto right = ((j + 1) > col_max) ? 0 : degree_count_label[z][i][j + 1];
					auto top_right = ((j + 1 <= col_max) && (i + 1) <= row_max) ? degree_count_label[z][i + 1][j + 1] : 0;
					degree_count_label[z][i][j] += top + right - top_right;
				}
			}
		}

		double* possibility = new double[graph.size()];
		pair<NodeIDType, pair<double, size_t>>* sortPoss = new pair<NodeIDType, pair<double, size_t>>[graph.size()];

		unordered_set<NodeIDType> notInSeq;
		for (auto& node : graph.nodes()) {
			auto id = node.id();
			notInSeq.insert(id);
			auto label = node.label();
			//	cout << degree_count_label[label][node.inEdgesNum()][node.outEdgesNum()] << endl;
			auto w1 = degree_count_label[label];
			auto w2 = w1[node.inEdgesNum()];
			auto w3 = w2[node.outEdgesNum()];
			possibility[id] = (double)(w3) / (targetGraph.size());

			sortPoss[id] = pair<NodeIDType, pair<double, size_t>>(id, pair<double, size_t>(possibility[id], node.outEdgesNum() + node.inEdgesNum()));

		}

		sort(sortPoss, sortPoss + graph.size(), [](const pair<NodeIDType, pair<double, size_t>>& a, const pair<NodeIDType, pair<double, size_t>>& b)
			{
				if (fabs(a.second.first - b.second.first) < 1E-30) {
					return a.second.second > b.second.second;
				}
				return a.second.first < b.second.first;
			});

		size_t maxDegreeInSeq = 0;
		map<NodeIDType, int> connect_num;
		auto* sortPossPoint = sortPoss;
		int seqID = -1;
		auto chooseNode = [&]() {
			int nowid = -1;
			double nowposs = 2;
			if (connect_num.size()) {
				auto answer_it = connect_num.begin();
				auto test_it = connect_num.begin()++;
				while (test_it != connect_num.end()) {
					if (test_it->second > answer_it->second) answer_it = test_it;
					else if (test_it->second < answer_it->second);
					else {
						auto answer_id = answer_it->first;
						auto test_id = test_it->first;
						if (fabs(possibility[answer_id] - possibility[test_id] < 1e-20)) {
							if (graph[answer_id].inEdgesNum() + graph[answer_id].outEdgesNum() < graph[test_id].inEdgesNum() + graph[test_id].outEdgesNum()) answer_it = test_it;
						}
						else {
							if (possibility[answer_id] > possibility[test_id]) answer_it = test_it;
						}
					}
					test_it++;
				}
				return (int)answer_it->first;
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
			connect_num.erase(seqID);
			if (notInSeq.empty())break;
			auto& node = graph.node(seqID);
			for (auto& edge : node.inEdges()) {
				auto temp = edge.source();
				if (IN_SET(notInSeq, temp)) 
					connect_num[temp]++;
			}
			for (auto& edge : node.outEdges()) {
				auto temp = edge.target();
				if (IN_SET(notInSeq, temp)) 
					connect_num[temp]++;
				
			}

			seqID = chooseNode();
		} while (true);
		delete[]sortPoss;
		delete[]possibility;
		return matchSequence;
	}
};


}