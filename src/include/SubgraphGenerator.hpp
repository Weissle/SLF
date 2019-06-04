#pragma once
#include<random>
#include<vector>
#include<assert.h>
#include<unordered_set>
#include<unordered_map>
#include<time.h>
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
		SubgraphGenerator<GraphType>(const GraphType &target, uint32_t _nN) : bigGraph(target), nodeNum(_nN) {
			srand(time(NULL));
			midG.reserve(nodeNum);
			inSmall.reserve(nodeNum << 1);
			inquery.reserve(target.graphSize() << 1);
			const auto temp = (size_t)(rand() % nodeNum);
			int i = 0;
			for (const auto &node : target.getAllNodes()) {
				if (i == temp) {
					inquery.insert(node.getID());
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
			vector<NodeType> smallGraphNodes;
			smallGraphNodes.reserve(num);
			for (auto i = 0; i < num; ++i) {
				const auto protoNode = bigGraph.getNode(midG[i]);
				const auto node = NodeType(i, protoNode.getLabel());
				smallGraphNodes.push_back(node);
			}
			smallGraph = GraphType(smallGraphNodes);
			for (auto i = 0; i < midG.size(); ++i) {
				const auto protoNode = bigGraph.getNode(midG[i]);
				const auto sourceID = index[midG[i]];
				for (auto edge : protoNode.getOutEdges()) {
					const auto pair = index.find(edge.getTargetNodeID());
					if (pair == index.end())continue;
					const auto targetID = pair->second;
					if (inSmall.find(pair->first) != inSmall.end())
						smallGraph.addEdge(sourceID, targetID, edge.getLabel());
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
