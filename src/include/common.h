#pragma once
#include<unordered_map>
size_t calHashSuitableSize(const size_t need) 
{
	size_t i = 8;
	while (i < need) i = i << 1;
	if (i * 0.8 > need) return i;
	else return i << 1;
};

template<typename K,typename V >
const V & getMapValue_C(const unordered_map<K ,V> &m,const K &k) {
	const auto& p = m.find(k);
	if (p == m.end())return V();
	else return p->second;
}


#define getMapValue(M,K) M[K]