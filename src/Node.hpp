#pragma once
#include<xhash>

class Node {
public:

	virtual bool operator==(const Node &n)const {
		return true;
	}

	virtual size_t nodeIdHash()const { return 0; }
};
namespace std
{
	template<>
	struct hash<Node>
	{
		size_t operator() (const Node  &n) const {
			return n.nodeIdHash();
		}
	};
}
