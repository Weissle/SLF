#pragma once
#include"Pair.hpp"
#include<algorithm>
#include<vector>
#include<iostream>
#include<string>
template<typename _NodeIDType>
class AnswerReceiver {
	typedef _NodeIDType NodeIDType;
	typedef KVPair<NodeIDType, NodeIDType> MapPair;
	std::fstream f;
public:
	AnswerReceiver() = default;
	AnswerReceiver(std::string SolutionPath)
	{
		f.open("D:\\data\\vsProject\\subgraph-isomorphism\\build\\Solutions.txt", std::ios_base::out);
		if (f.is_open() == false) cout << "solution file open fail" << endl;
	}
	~AnswerReceiver() = default;
	template<typename MapType>
	void operator<<(const MapType &mapping) {
		static size_t count = 1;
		
		if (f.is_open()) {
			f << "solution " << count  << endl;
			f << mapping.size() << endl;
			for (auto i = 0; i < mapping.size(); ++i) {
				f << i << ' ' << mapping[i] << endl;
			}
		}
		
		std::cout << "Solution : " << count++ << std::endl;
		for (auto i = 0; i < mapping.size();++i) {

			std::cout << '(' << i << ',' << mapping[i] << ") ";
		}
	/*	std::vector<MapPair> v;
		v.reserve(mapping.size());
		for (const auto p : mapping) {
			v.push_back(MapPair(p.first, p.second));
		}
		std::sort(v.begin(), v.end(), [](const MapPair &a, const MapPair &b) {
			return a < b;
		});
		for (const auto &pair : v) {
			std::cout << '(' << pair.getKey() << ',' << pair.getValue() << ") ";
		}*/
		std::cout << std::endl;
		return;
	}
	void finish() {
		f.close();
	}

};