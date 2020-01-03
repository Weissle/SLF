#pragma once
#include<unordered_set>
#include<vector>
#include<set>
#include<map>
#include"Graph.hpp"
#include"Pair.hpp"
using namespace std;
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
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + ioMap[node.id()] * 2;
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set& s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {
				const auto& tempNode = graph.getNode(tempNodeID);
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

			for (const auto& tempEdge : node.getInEdges()) {
				const auto sourceNodeID = tempEdge.getSourceNodeID();
				if (IN_SET(nodeNotInMatchSet, sourceNodeID)) {
					ioSet.insert(sourceNodeID);
					ioMap[sourceNodeID]++;

				}
			}
			for (const auto& tempEdge : node.getOutEdges()) {
				const auto targetNodeID = tempEdge.getTargetNodeID();
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
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + ioMap[node.id()] * 2;
			for (const auto& edge : node.getInEdges()) {
				const auto nodeID = edge.getSourceNodeID();
				const auto& tempNode = graph.getNode(nodeID);
				p1 += tempNode.getInEdgesNum() + tempNode.getOutEdgesNum();
			}
			for (const auto& edge : node.getOutEdges()) {
				const auto nodeID = edge.getTargetNodeID();
				const auto& tempNode = graph.getNode(nodeID);
				p1 += tempNode.getInEdgesNum() + tempNode.getOutEdgesNum();
			}
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set& s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {

				const auto& tempNode = graph.getNode(tempNodeID);
				//			if (ioMap[tempNodeID] == tempNode.getInEdgesNum() + tempNode.getOutEdgesNum()) return NodeMatchPointPair(&tempNode, nodePoint);
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

			for (const auto& tempEdge : node.getInEdges()) {
				const auto sourceNodeID = tempEdge.getSourceNodeID();
				if (IN_SET(nodeNotInMatchSet, sourceNodeID)) {
					ioSet.insert(sourceNodeID);
					ioMap[sourceNodeID]++;

				}
			}
			for (const auto& tempEdge : node.getOutEdges()) {
				const auto targetNodeID = tempEdge.getTargetNodeID();
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
		map<NodeLabelType, size_t> pgLQ = graph.getLQinform(), tgLQ = targetGraph.getLQinform();
		map<NodeLabelType, FSPair<size_t, size_t>>  pgLD = graph.getLDinform(), tgLD = targetGraph.getLDinform();
		vector< vector< size_t > > pgin, pgout, tgin, tgout;
		assert(pgLQ.size() <= tgLQ.size() && " pattern graph is not a subgraph of target graph owing to the label type");
		//	pgin.resize(pgLQ.size());
		tgin.resize(pgLQ.size());
		//	pgout.resize(pgLQ.size());
		tgout.resize(pgLQ.size());
		LOOP(i, 0, pgLQ.size()) {
			/*	pgin[i].resize(pgLD[i].second+2);
				pgout[i].resize(pgLD[i].first +2);*/
			tgin[i].resize(tgLD[i].second + 2);
			tgout[i].resize(tgLD[i].first + 2);
			assert(pgLD[i].second <= tgLD[i].second && pgLD[i].first <= tgLD[i].first && " pattern graph is not a subgraph of target graph owing to in or out degree");

		}

		for (auto& node : targetGraph.nodes()) {
			if (node.getLabel() >= pgLQ.size()) continue;
			tgin[node.getLabel()][node.getInEdgesNum()]++;
			tgout[node.getLabel()][node.getOutEdgesNum()]++;
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
		map<NodeIDType, size_t> aux;
		double* possibility = new double[graph.size()];
		FSPair<NodeIDType, FSPair<double, size_t>>* sortPoss = new FSPair<NodeIDType, FSPair<double, size_t>>[graph.size()];

		set<NodeIDType> notInSeq;
		for (auto node : graph.nodes()) {
			auto id = node.id();
			notInSeq.insert(id);
			auto label = node.getLabel();
			possibility[id] = (double)(tgin[label][node.getInEdgesNum()] * tgout[label][node.getOutEdgesNum()]) / (targetGraph.size() * targetGraph.size());

			sortPoss[id] = FSPair<NodeIDType, FSPair<double, size_t>>(id, FSPair<double, size_t>(possibility[id], node.getOutEdgesNum() + node.getInEdgesNum()));
		//	sortPoss[id] = FSPair<NodeIDType, FSPair<double, size_t>>{ id, FSPair<double, size_t>{possibility[id], node.getOutEdgesNum() + node.getInEdgesNum()} };
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
							auto nownode = graph.getNode(nowid);
							auto thisnode = graph.getNode(it->first);
							if (nownode.getOutEdgesNum() + nownode.getInEdgesNum() < thisnode.getOutEdgesNum() + thisnode.getInEdgesNum()) {
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
			auto node = graph.getNode(seqID);
			for (auto& edge : node.getInEdges()) {
				auto temp = edge.getSourceNodeID();
				if (IN_SET(notInSeq, temp)) ioMap[temp]++;
			}
			for (auto& edge : node.getOutEdges()) {
				auto temp = edge.getTargetNodeID();
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