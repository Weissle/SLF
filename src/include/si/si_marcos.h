#pragma once
#include <limits>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


#define INDUCE_ISO
//#define NORMAL_ISO
//#define WG_MEMORY_SAVE


namespace wg {

typedef uint32_t NodeIDType;
using NodeLabel = uint32_t;
using MapPair = std::pair<NodeIDType, NodeIDType>;
using MapType = std::vector<NodeIDType>;
//constexpr size_t NO_MAP = SIZE_MAX;
constexpr NodeIDType NO_MAP = std::numeric_limits<NodeIDType>::max();
constexpr MapPair error_pair = MapPair(NO_MAP, NO_MAP);

}


