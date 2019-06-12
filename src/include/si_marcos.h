#pragma once

// NodeSet class

//#define TRAVERSE_SET(TEMP_VARIABLE_NAME,S) for(const auto &TEMP_VARIABLE_NAME=S.begin();TEMP_VARIABLE_NAME!=S.end();++TEMP_VARIABLE_NAME)
#define TRAVERSE_SET(TEMP_VARIABLE_NAME,S) for(const auto &TEMP_VARIABLE_NAME:S)
// unordered_set class
#define IN_SET(S,K) (S.find(K)!=S.end())
#define NOT_IN_SET(S,K) (S.find(K)==S.end())



