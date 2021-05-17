#pragma once
#include <algorithm>
#include <cstdint>
#include<vector>
#include"Node.hpp"
#include"Edge.hpp"
#include"si/si_marcos.h"
#include<unordered_map>
#include<assert.h>
using namespace std;
namespace wg {
template<typename _Node, typename _EdgeLabelType>
class Graph
{
public:
	enum GRAPH_TYPE { BIDIRECTION, DIRECTION };
	using NodeType = _Node;
	using NodeLabelType = typename NodeType::NodeLabelType;
	using EdgeLabelType = _EdgeLabelType;
	typedef Graph<NodeType, _EdgeLabelType> GraphType;
private:

	vector<NodeType> _nodes;
	GRAPH_TYPE graphType;
	size_t _size;

	// NodeLabel -> the maximum out and in degrees of this kind of nodes have;
	unordered_map<NodeLabelType, size_t> aux_LabelMaxOut;
	unordered_map<NodeLabelType, size_t> aux_LabelMaxIn;
	// NodeLabel -> quantity of nodes have this label
	unordered_map<NodeLabelType, size_t> aux_LabelNum;
	NodeLabelType _maxLabel = 0;
	Graph() = default;
public:
	Graph(const size_t s, GRAPH_TYPE _graphType = GRAPH_TYPE::DIRECTION) :_size(s), graphType(_graphType) {
		vector<NodeType> n;
		n.reserve(s + 1);
		for (auto i = 0; i < s; ++i) n.push_back(NodeType(i));
		swap(_nodes, n);
	}

	void edgeVectorReserve(const NodeIDType id, size_t s) {
		assert(id < _size);
		_nodes[id].reserve(s);
		return;
	}
	void setNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		assert((_id < _size) && "node ID overflow");
		_nodes[_id].setLabel(_label);
	}
	void addEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel = EdgeLabelType()) {

		//	assert(source != target && "not support self loop");
		assert(source < _size && target < _size && "node id should smaller than node number");
		auto& sourceNode = _nodes[source];
		auto& targetNode = _nodes[target];

		sourceNode.addOutEdge(target,edgeLabel);
		targetNode.addInEdge(source,edgeLabel);
		if (GRAPH_TYPE::BIDIRECTION == graphType && source != target) {
			sourceNode.addInEdge(target,edgeLabel);
			targetNode.addOutEdge(source,edgeLabel);
		}
	}

	size_t size() const {
		return _size;
	};
	const vector<NodeType>& nodes()const { return _nodes; }

	const NodeType& node(const NodeIDType& nodeID) const {
		assert((nodeID < _size) && "node ID overflow");
		return _nodes[nodeID];
	}
	const unordered_map<NodeLabelType,size_t>& labelMaxIn()const {
		return this->aux_LabelMaxIn;
	}
	const unordered_map<NodeLabelType, size_t>& labelMaxOut()const {
		return this->aux_LabelMaxOut;
	}
	const unordered_map<NodeLabelType, size_t>& labelNum()const {
		return this->aux_LabelNum;
	}
	//do something to improve match speed
	void graphBuildFinish() {


		//only allow label type from 0 1 2 3 ... n-1
		for (auto& node : _nodes) {
			node.NodeBuildFinish();
			auto nodeLabel = node.label();
			this->aux_LabelNum[nodeLabel]++;

			auto &inMax = aux_LabelMaxIn[nodeLabel];
			inMax = max(inMax, node.inEdgesNum());
			auto& outMax = aux_LabelMaxOut[nodeLabel];
			outMax = max(outMax, node.outEdgesNum());
			_maxLabel = max(_maxLabel, nodeLabel);
		}

	}
	NodeLabelType maxLabel()const { return _maxLabel; }
	bool existEdge(const NodeIDType& from, const NodeIDType& to, const EdgeLabelType& edgeLabel)const {
		return _nodes[from].existSameTypeEdgeToNode(to, edgeLabel);
	}
	const NodeType& operator[](const NodeIDType& nodeID)const {
		return node(nodeID);
	}

};

//template<typename _NodeLabelType,typename _EdgeLabelType>
template<typename _EdgeLabelType>
class GraphS{
public:
	using EdgeLabelType = _EdgeLabelType;
	using NodeLabelType = uint32_t;
	using TEdge = TargetEdge<EdgeLabelType>;
	using SEdge = SourceEdge<EdgeLabelType>;
private:
	vector<NodeLabelType> labels;
	vector<vector<TEdge>> tedges;
	vector<vector<SEdge>> sedges;
	size_t _size;

	GraphS()=default;
	GraphS(const GraphS&)=delete;
public:
	GraphS(const size_t s) :_size(s){
		labels.resize(s);
		tedges.resize(s);
		sedges.resize(s);
	}
	void SetNodeLabel(const NodeIDType _id, const NodeLabelType _label) {
		assert((_id >= 0 && _id < _size) && "node ID overflow");
		labels[_id] = _label;
	}
	void AddEdge(const NodeIDType source, const NodeIDType target, const EdgeLabelType edgeLabel = EdgeLabelType()) {
		//	assert(source != target && "not support self loop");
		assert(source < _size && target < _size && "node id should smaller than node number");
		tedges[source].emplace_back(TEdge(target,edgeLabel));
		sedges[target].emplace_back(SEdge(source,edgeLabel));
	}
	size_t Size() const {
		return _size;
	};

	void SortEdge() {
		//only allow label type from 0 1 2 3 ... n-1
		for (int i = 0; i < _size; ++i){ 
			sort(tedges[i].begin(), tedges[i].end());
			sort(sedges[i].begin(), sedges[i].end());
		}
	}
	bool ExistEdge(const NodeIDType& from, const NodeIDType& to, const EdgeLabelType& edgeLabel)const {
		const TEdge t(to,edgeLabel);
		return binary_search(tedges[from].begin(), tedges[from].end(),t);
	}
	const vector<TEdge>& GetOutEdges(int id) const { return tedges[id]; }
	const vector<SEdge>& GetInEdges(int id) const { return sedges[id]; }
	int GetInDegree(int id) const { return sedges[id].size(); }
	int GetOutDegree(int id) const { return tedges[id].size(); }
	const vector<NodeLabelType>& GetLabels() const { return labels; }
	const NodeLabelType GetLabel(int id) const { return labels[id]; }

};
}
