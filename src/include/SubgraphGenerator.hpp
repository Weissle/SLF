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
			const auto temp = (size_t)((uint32_t)rand() % bigGraph.size());
			inquery.insert(temp);
			index.reserve(nodeNum << 1);
		}
		void run() {
			int inSmallSize = 0;
			while (inSmallSize < nodeNum && inquery.empty() == false) {
				int i = 0, temp;
				int j = ((uint32_t)rand()) % inquery.size();
				for (auto k : inquery) {
					i++;
					temp = k;
					if (i < j)continue;
				}
				const auto node = bigGraph.node(temp);
				for (const auto edge : node.inEdges()) {
					const auto ver = edge.source();
					if (inSmall.find(ver) == inSmall.end()) {
						inquery.insert(ver);
					}
				}
				for (const auto edge : node.outEdges()) {
					const auto ver = edge.target();
					if (inSmall.find(ver) == inSmall.end()) {
						inquery.insert(ver);
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

			IndexTurner<size_t> turner(nodeNum);
			for (auto i = 0; i < midG.size(); ++i) {
				turner(midG[i]);
			}
			smallGraph = GraphType(nodeNum);

			for (auto i = 0; i < midG.size(); ++i) {
				unordered_set<size_t> s;
				s.reserve(midG.size());
				const auto &protoNode = bigGraph.node(midG[i]);
				const auto sourceID = turner(protoNode.id());
				smallGraph.setNodeLabel(sourceID, protoNode.label());
				for (auto edge : protoNode.outEdges()) {
					const auto targetID = edge.target();
					if (turner.exist(targetID) == false) continue;
					if (IN_SET(s, targetID)) {
						int a=0;
						cout << "sgfesgsgsegs" << endl;
					}
					smallGraph.addEdge(sourceID, turner(targetID), edge.label());	
					s.insert(targetID);
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
