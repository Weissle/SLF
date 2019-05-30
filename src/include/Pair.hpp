#pragma once

template<typename K, typename V>
class KVPair {
	K key;
	V value;
public:
	KVPair() = default;
	KVPair(K _k, V _v) :key(_k), value(_v) {};
	~KVPair() = default;	
	K getKey()const { return key; }
	V getValue()const { return value; }
	inline bool operator==(const KVPair<K, V> &p) const {
		return p.getKey() == key;
	}
	bool operator<(const KVPair &p)const {
		return key < p.getKey();
	}
	bool operator>(const KVPair &p)const {
		return key > p.getKey();
	}
};
namespace std {
	template<typename K, typename V>
	struct hash<KVPair<K, V> >
	{
		size_t operator()(const KVPair<K, V> &p)const {
			return hash<K>()(p.getKey());
		}
	};
}

template<typename F, typename S>
class FSPair {
	F first;
	S second;
public:
	F getFirst()const { return first; }
	S getSecond()const { return second; }
	FSPair() = default;
	~FSPair() = default;
	FSPair(F _f, S _s) :first(_f),second(_s) {};
	inline bool operator==(const FSPair<F, S> &p) const {
		return (p.getFirst() == first && p.getSecond() == second);
	}
};
namespace std {
	template<typename F, typename S>
	struct hash<FSPair<F, S> >
	{
		size_t operator()(const FSPair<F, S> &p)const {
			auto hash1 = hash<F>()(p.getFirst());
			auto hash2 = hash<S>()(p.getValue());
			return ((hash1 << 2) + 0x9e3779b9) ^ (hash2);
		}
	};
}
