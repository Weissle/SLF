#pragma once
#include<xhash>

class Node {
public:

	//should id equal
	virtual bool operator==(const Node &n)const {
		return true;
	}
	//same label
	virtual bool isSameType(const Node &n)const {
		return true;
	}
	static bool isSameTypeNode(const Node &n1, const Node &n2) {
		return n1.isSameType(n2);
	}
	// something like edge
	virtual bool operator>=(const Node &n) const{
		return true;
	}
	virtual bool operator<=(const Node &n)const {
		return true;
	}
//	virtual bool 
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
