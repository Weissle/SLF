#pragma once

template<typename NodeIDType>
class Edge {
public:
	Edge() = default;
	~Edge() = default;
	virtual const NodeIDType& getSourceNode() const { return NodeIDType(); }
	virtual const NodeIDType& getTargetNode()const { return NodeIDType(); }
	virtual bool isSameTypeEdge(const Edge<NodeIDType> &n) const { return true; }
	virtual bool operator==(const Edge<NodeIDType> &e) {
		return true;
	}
};

template<typename NodeIDType>
class SourceEdge : Edge<NodeIDType> {
	NodeIDType source;

public:
	SourceEdge(NodeIDType _source):source(_source){}
	SourceEdge() = default;
	~SourceEdge() = default;
	virtual const NodeIDType& getSourceNode() const { return source; }
	virtual const NodeIDType& getTargetNode()const 
	{ 
		throw "this is source edge!!";
		return NodeIDType();
	}
};
template<typename NodeIDType>
class TargetEdge : Edge<NodeIDType> {
	NodeIDType target;

public:
	TargetEdge(NodeIDType _target) :target(_target) {}
	TargetEdge() = default;
	~TargetEdge() = default;
	virtual const NodeIDType& getSourceNode() const 
	{ 
		throw "this is targer edge !!";
		return NodeIDType();
	}
	virtual const NodeIDType& getTargetNode()const
	{
		
		return target;
	}
};
