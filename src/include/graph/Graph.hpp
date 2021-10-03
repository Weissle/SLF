#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
#include "Edge.hpp"
#include "si/si_marcos.h"
#include <unordered_map>
#include <assert.h>
using namespace std;
namespace wg {

template<typename EdgeLabel>
class GraphS{
public:
	using TEdge = TargetEdge<EdgeLabel>;
	using SEdge = SourceEdge<EdgeLabel>;
private:
	vector<wg::NodeLabel> labels;
	vector<vector<TEdge>> tedges;
	vector<vector<SEdge>> sedges;
	size_t _size;

	GraphS()=default;
	// copy is not allow.
	GraphS(const GraphS&)=delete;
public:
	GraphS(const size_t s) :_size(s){
		labels.resize(s);
		tedges.resize(s);
		sedges.resize(s);
	}
	void SetNodeLabel(const NodeIDType _id, const NodeLabel _label) {
		//only allow label type from 0 1 2 3 ... n-1
		assert((_id >= 0 && _id < _size) && "node ID overflow");
		labels[_id] = _label;
	}
	void AddEdge(const NodeIDType source, const NodeIDType target, const EdgeLabel edgeLabel = EdgeLabel()) {
		//	assert(source != target && "not support self loop");
		assert(source < _size && target < _size && "node id should smaller than node number");
		tedges[source].emplace_back(TEdge(target,edgeLabel));
		sedges[target].emplace_back(SEdge(source,edgeLabel));
	}
	size_t Size() const {
		return _size;
	};

	// Edges with same source, target and label only leave one.
	void SortEdge() {
		for (int i = 0; i < _size; ++i){ 
			sort(tedges[i].begin(), tedges[i].end());
			sort(sedges[i].begin(), sedges[i].end());
			auto tit = unique(tedges[i].begin(),tedges[i].end());
			tedges[i].erase(tit,tedges[i].end());
			auto sit = unique(sedges[i].begin(),sedges[i].end());
			sedges[i].erase(sit,sedges[i].end());
		}
	}

	bool ExistEdge(const NodeIDType& from, const NodeIDType& to, const EdgeLabel& edgeLabel)const {
		const TEdge t(to,edgeLabel);
		return binary_search(tedges[from].begin(), tedges[from].end(),t);
	}

	const vector<TEdge>& GetOutEdges(int id) const { return tedges[id]; }
	const vector<SEdge>& GetInEdges(int id) const { return sedges[id]; }
	int GetInDegree(int id) const { return sedges[id].size(); }
	int GetOutDegree(int id) const { return tedges[id].size(); }
	const vector<NodeLabel>& GetLabels() const { return labels; }
	const NodeLabel GetLabel(int id) const { return labels[id]; }
	
	// To reserve all vector to avoid cost too much time on new and delete array.(This is an option and is not necessary)
	void reserve(int n){
		for (int i = 0; i < _size; ++i){ 
			tedges[i].reserve(n);
			sedges[i].reserve(n);
		}
	}
};
}
