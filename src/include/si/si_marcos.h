#pragma once
#include <limits>
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
//#define WG_MEMORY_SAVE


namespace wg {

typedef uint32_t NodeIDType;
using MapPair = std::pair<NodeIDType, NodeIDType>;
using MapType = std::vector<NodeIDType>;
//constexpr size_t NO_MAP = SIZE_MAX;
constexpr NodeIDType NO_MAP = std::numeric_limits<NodeIDType>::max();
constexpr MapPair error_pair = MapPair(NO_MAP, NO_MAP);

}


