#pragma once
#include<random>
#include<vector>
#include<assert.h>
#include<unordered_set>
#include<unordered_map>
#include<time.h>
#include<IndexTurner.hpp>
#define FLOAT_ZERO 1E-6
namespace sg {


	template<typename GraphType>
	class SubgraphGenerator {
		typedef typename GraphType::NodeType NodeType;
		typedef typename NodeType::NodeIDType NodeIDType;
		typedef typename GraphType::EdgeType EdgeType;

		unordered_set<NodeIDType> inSmall, inquery;
		const GraphType &bigGraph;
		unordered_map<NodeIDType, NodeIDType> index;
		GraphType smallGraph;
		uint32_t nodeNum;
		vector<NodeIDType> midG;
	public:
		SubgraphGenerator<GraphType>(const GraphType &target, size_t _nN) : bigGraph(target), nodeNum(_nN) {
			srand(time(NULL));
			midG.reserve(nodeNum);
			inSmall.reserve(nodeNum );
			inquery.reserve(target.size() << 1);
			const auto temp = (size_t)((uint32_t)rand() % nodeNum);
			int i = 0;
			for (const auto &node : target.nodes()) {
				if (i == temp) {
					inquery.insert(node.id());
					break;
				}
				else ++i;
			}
		}
		void run() {
			int inSmallSize = 0;
			int inquerySize;
			while (inSmallSize < nodeNum && inquery.empty() == false) {
				int i = 0, temp;
				int j = ((uint32_t)rand()) % inquery.size();
				for (auto k : inquery) {
					i++;
					temp = k;
					if (i < j)continue;
				}
				if (inquery.size() + inSmallSize < nodeNum) {
	
					const auto node = bigGraph.getNode(temp);
					for (const auto edge : node.getInEdges()) {
						if (inquery.size() + inSmallSize > nodeNum)break;
						const auto ver = edge.getSourceNodeID();
						if (inSmall.find(ver) == inSmall.end()) {
							inquery.insert(ver);
						}
					}
					for (const auto edge : node.getOutEdges()) {
						if (inquery.size() + inSmallSize > nodeNum)break;
						const auto ver = edge.getTargetNodeID();
						if (inSmall.find(ver) == inSmall.end()) {
							inquery.insert(ver);
						}
					}
				}
				inSmall.insert(temp);
				++inSmallSize;
				inquery.erase(temp);

			}
			NodeIDType num = 0;
			for (const auto nodeID : inSmall) {
				index[nodeID] = num;
				midG.push_back(nodeID);
				++num;
			}
/*			vector<NodeType> smallGraphNodes;
			smallGraphNodes.reserve(num);
			for (auto i = 0; i < num; ++i) {
				const auto protoNode = bigGraph.getNode(midG[i]);
				const auto node = NodeType(i, protoNode.getLabel());
				smallGraphNodes.push_back(node);
			}*/

			IndexTurner<size_t> turner(nodeNum);
			for (auto i = 0; i < midG.size(); ++i) {
				turner(midG[i]);
			}
			smallGraph = GraphType(nodeNum);

			for (auto i = 0; i < midG.size(); ++i) {
				unordered_set<size_t> s;
				s.reserve(midG.size());
				const auto protoNode = bigGraph.getNode(midG[i]);
				const auto sourceID = turner(protoNode.id());
				smallGraph.setNodeLabel(sourceID, protoNode.getLabel());
				for (auto edge : protoNode.getOutEdges()) {
					const auto pair = index.find(edge.getTargetNodeID());
					if (pair == index.end())continue;

					const auto targetID = pair->second;
					if (inSmall.find(pair->first) != inSmall.end()) {
					/*	if (sourceID == 0 && targetID == 41) {
							int a = 0;

						}*/
						smallGraph.addEdge(sourceID, targetID, edge.getLabel());
					}
				}

			}
			return;
		}
		vector<NodeIDType> getMid() {
			return midG;
		}
		GraphType getSmallGraph() {
			return smallGraph;


		}

	};
}
