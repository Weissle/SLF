#pragma once
#include<unordered_set>
#include<vector>
#include<set>
#include<map>
#include"Graph.hpp"
#include"Pair.hpp"
#include<utility>
using namespace std;
namespace wg {
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
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
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
			assert(nodePointPair.getKey() != nullptr && "error happened");
			const auto& node = *nodePointPair.getKey();

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
		for (const auto node : graph.nodes()) {
			nodeNotInMatchSet.insert(node.id());
		}
		Set ioSet;

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
			assert(nodePointPair.getKey() != nullptr && "error happened");
			const auto& node = *nodePointPair.getKey();

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
class MatchOrderSelectorVF3 {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	typedef typename NodeType::NodeLabelType NodeLabelType;
	static vector<NodeIDType> run(const GraphType& graph, const GraphType& targetGraph) {
		vector<NodeIDType> matchSequence;
		matchSequence.reserve(graph.size() + 1);
		unordered_map<NodeLabelType, size_t> pgLQ = graph.LQinform(), tgLQ = targetGraph.LQinform();
		unordered_map<NodeLabelType, pair<size_t, size_t>>  pgLD = graph.LDinform(), tgLD = targetGraph.LDinform();
		vector< vector< size_t > > tgin, tgout;
		assert(pgLQ.size() <= tgLQ.size() && " pattern graph is not a subgraph of target graph owing to the label type");

		tgin.resize(pgLQ.size());

		tgout.resize(pgLQ.size());
		LOOP(i, 0, pgLQ.size()) {

			tgin[i].resize(tgLD[i].second + 2);
			tgout[i].resize(tgLD[i].first + 2);
			assert(pgLD[i].second <= tgLD[i].second && pgLD[i].first <= tgLD[i].first && " pattern graph is not a subgraph of target graph owing to in or out degree");

		}

		for (auto& node : targetGraph.nodes()) {
			if (node.label() >= pgLQ.size()) continue;
			tgin[node.label()][node.inEdgesNum()]++;
			tgout[node.label()][node.outEdgesNum()]++;
		}
		LOOP(i, 0, pgLQ.size()) {
			size_t nodeCount = tgLQ[i];
			LOOP(j, 0, tgout[i].size()) {
				int temp = tgout[i][j];
				tgout[i][j] = nodeCount;
				nodeCount -= temp;
			}
			nodeCount = tgLQ[i];
			LOOP(j, 0, tgin[i].size()) {
				int temp = tgin[i][j];
				tgin[i][j] = nodeCount;
				nodeCount -= temp;
			}
		}
		double* possibility = new double[graph.size()];
		FSPair<NodeIDType, FSPair<double, size_t>>* sortPoss = new FSPair<NodeIDType, FSPair<double, size_t>>[graph.size()];

		unordered_set<NodeIDType> notInSeq;
		for (auto node : graph.nodes()) {
			auto id = node.id();
			notInSeq.insert(id);
			auto label = node.label();
			possibility[id] = (double)(tgin[label][node.inEdgesNum()] * tgout[label][node.outEdgesNum()]) / (targetGraph.size() * targetGraph.size());

			sortPoss[id] = FSPair<NodeIDType, FSPair<double, size_t>>(id, FSPair<double, size_t>(possibility[id], node.outEdgesNum() + node.inEdgesNum()));

		}

		sort(sortPoss, sortPoss + graph.size(), [](const FSPair<NodeIDType, FSPair<double, size_t>>& a, const FSPair<NodeIDType, FSPair<double, size_t>>& b)
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
}