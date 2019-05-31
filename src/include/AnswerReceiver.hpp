#pragma once
#include"Pair.hpp"
#include<algorithm>
#include<vector>
#include<iostream>

template<typename _NodeIDType>
class AnswerReceiver {
	typedef _NodeIDType NodeIDType;
	typedef KVPair<NodeIDType, NodeIDType> MapPair;
public:
	AnswerReceiver() = default;
	~AnswerReceiver() = default;
	template<typename MapType>
	void operator<<(const MapType &mapping) {
		static size_t count = 1;
		std::cout << "Solution : " << count++ << std::endl;
		std::vector<MapPair> v;
		v.reserve(mapping.size());
		for (const auto p : mapping) {
			v.push_back(MapPair(p.first, p.second));
		}
		std::sort(v.begin(), v.end(), [](const MapPair &a, const MapPair &b) {
			return a < b;
		});
		for (const auto &pair : v) {
			std::cout << '(' << pair.getKey() << ',' << pair.getValue() << ") ";
		}
		std::cout << std::endl;
		return;
	}

};