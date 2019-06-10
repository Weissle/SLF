#pragma once

// NodeSet class
#define IN_NODESET(S,ID) (S.exist(ID))
#define NOT_IN_NODESET(S,ID) (!S.exist(ID))
#define TRAVERSE_NODESET(TEMP_VARIABLE_NAME,S) for(const auto & TEMP_VARIABLE_NAME : S.getSet())
// unordered_set class
#define IN_SET(S,ID) (S.find(ID)!=S.end())
#define NOT_IN_SET(S,ID) (S.find(ID)==S.end())



