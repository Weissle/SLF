#pragma once
#include<unordered_set>
#include<vector>
#include<set>
#include"Graph.hpp"
#include"Pair.hpp"
using namespace std;
template<class _GraphType>
class MatchOrderSelector {
public:
	typedef _GraphType GraphType;
	typedef typename GraphType::NodeType NodeType;
	typedef typename NodeType::NodeIDType NodeIDType;
	static vector<NodeIDType> run(const GraphType &graph) {
		typedef set<NodeIDType> Set;
		Set nodeNotInMatchSet;
		for (const auto node : graph.nodes()) {
			nodeNotInMatchSet.insert(node.id);
		}
		Set ioSet;

		typedef const NodeType* NodeCPointer;
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		vector<int>  ioMap;

		ioMap.resize(graph.size() + 1);

		const auto calNodeMatchPoint = [&](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + ioMap[node.id] * 2;
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set & s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {
				const auto & tempNode = graph.getNode(tempNodeID);
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
			const auto & node = *nodePointPair.getKey();

			const auto nodeID = node.id;
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
	static vector<NodeIDType> run(const GraphType &graph) {
		typedef set<NodeIDType> Set;
		Set nodeNotInMatchSet;
		for (const auto node : graph.nodes()) {
			nodeNotInMatchSet.insert(node.id);
		}
		Set ioSet;

		typedef const NodeType* NodeCPointer;
		typedef KVPair<NodeCPointer, double> NodeMatchPointPair;
		vector<int>  ioMap;

		ioMap.resize(graph.size() + 1);

		const auto calNodeMatchPoint = [&](const NodeType & node) {
			double p1 = node.getInEdgesNum() + node.getOutEdgesNum() + ioMap[node.id] * 2;
			for (const auto &edge : node.getInEdges()) {
				const auto nodeID = edge.getSourceNodeID();
				const auto &tempNode = graph.getNode(nodeID);
				p1 += tempNode.getInEdgesNum() + tempNode.getOutEdgesNum();
			}
			for (const auto &edge : node.getOutEdges()) {
				const auto nodeID = edge.getTargetNodeID();
				const auto &tempNode = graph.getNode(nodeID);
				p1 += tempNode.getInEdgesNum() + tempNode.getOutEdgesNum();
			}
			return p1;
		};
		const auto seleteAGoodNodeToMatch = [&](const Set & s) {
			double nodePoint = -1;
			NodeCPointer answer = nullptr;
			for (const auto& tempNodeID : s) {
				
				const auto & tempNode = graph.getNode(tempNodeID);
				if (ioMap[tempNodeID] == tempNode.getInEdgesNum() + tempNode.getOutEdgesNum()) return NodeMatchPointPair(&tempNode, nodePoint);
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
			const auto & node = *nodePointPair.getKey();

			const auto nodeID = node.id;
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