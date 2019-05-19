#pragma once

template<typename K, typename V>
class Pair {
	K key;
	V value;
public:
	Pair() = default;
	Pair(K _k, V _v) :key(_k), value(_v) {};
	~Pair() = default;
	K getFirst()const { return key; }
	K getKey()const { return key; }
	V getSecond()const { return value; }
	V getValue()const { return value; }
	inline bool operator==(const Pair<K, V> &p) const {
		return (p.getKey() == key && p.getValue() == value);
	}
	/*	inline static bool operator==()(const Pair<K, V> &p1, const Pair<K, V> &p2){
			return p1 == p2;
		}*/
};
namespace std {
	template<typename K, typename V>
	struct hash<Pair<K, V> >
	{
		size_t operator()(const Pair<K, V> &p)const {
			auto hash1 = hash<K>()(p.getFirst());
			auto hash2 = hash<V>()(p.getValue());
			return ((hash1 << 2) + 0x9e3779b9) ^ (hash2);
		}
	};
}
template<typename K, typename V>
class MappingPair : public Pair<K, V> {

public:
	MappingPair() = default;
	~MappingPair() = default;
	MappingPair(K _k, V _v) :Pair(_k, _v) {};
};