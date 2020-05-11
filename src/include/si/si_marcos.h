#pragma once
#include<utility>
#include<vector>
#include<string>
#include<iostream>
#include<fstream>

// unordered_set class
#define IN_SET(S,K) ((S).find(K)!=(S).end())
#define NOT_IN_SET(S,K) ((S).find(K)==(S).end())

#define INDUCE_ISO
//#define NORMAL_ISO
//#define PRINT_ALL_SOLUTIONS
//#define WG_MEMORY_SAVE


namespace wg {

typedef size_t NodeIDType;
using MapPair = std::pair<NodeIDType, NodeIDType>;
using  MapType = std::vector<NodeIDType>;
const size_t NO_MAP = SIZE_MAX;
const MapPair error_pair = MapPair(NO_MAP, NO_MAP);
std::vector<size_t> readMatchSequence(std::string& matchOrderPath) {
	std::vector<size_t> ms;
	if (matchOrderPath.empty() == false) {
		std::fstream f;
		f.open(matchOrderPath.c_str(), std::ios_base::in);
		if (f.is_open() == false) {
			std::cout << matchOrderPath << " open fail" << std::endl;
			exit(1);
		}
		while (f.eof() == false) {
			size_t temp = UINT32_MAX;
			f >> temp;
			if (temp != UINT32_MAX)ms.push_back(temp);
		}
		f.close();
	}
	return move(ms);
}
}


