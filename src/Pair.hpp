#pragma once

template<typename K,typename V>
class Pair {
	K key;
	V value;
public:
	Pair() = default;
	Pair(K _k, V _v) :key(_k), value(_v) {};
	~Pair() = default;
};
template<typename K, typename V>
class MappingPair : public Pair {

public:
	MappingPair() = default;
	~MappingPair() = default;
	MappingPair(K _k, V _v) :Pair(_k, _v) {};
};