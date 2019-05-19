#pragma once
#include<vector>
#include"Node.hpp"
using namespace std;
class Graph {

public:
	Graph() = default;
	~Graph() = default;
	virtual size_t graphSize() const {
		return 0;
	};
	virtual vector<Node> const & getAllNodes()const { return vector<Node>(); }
};
