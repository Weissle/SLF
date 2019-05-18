#pragma once

class Graph {

public:
	Graph() = default;
	~Graph() = default;
	virtual uint32_t graphSize () const { return 0; }
};