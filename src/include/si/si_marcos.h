#pragma once

// NodeSet class

//#define TRAVERSE_SET(TEMP_VARIABLE_NAME,S) for(const auto &TEMP_VARIABLE_NAME=S.begin();TEMP_VARIABLE_NAME!=S.end();++TEMP_VARIABLE_NAME)
#define TRAVERSE_SET(TEMP_VARIABLE_NAME,S) for(const auto &TEMP_VARIABLE_NAME:S)
// unordered_set class
#define IN_SET(S,K) ((S).find(K)!=(S).end())

#define IN_NODE_SET(S,K) ((S).exist(K))
#define NOT_IN_SET(S,K) ((S).find(K)==(S).end())

#define INDUCE_ISO
//#define NORMAL_ISO
namespace wg {
typedef size_t NodeIDType;
typedef pair<NodeIDType, NodeIDType> MapPair;
typedef vector<NodeIDType> MapType;
const size_t NO_MAP = SIZE_MAX;
const MapPair error_pair = MapPair(NO_MAP, NO_MAP);
}


