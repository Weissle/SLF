#include<iostream>
#include<unordered_set>
#include<algorithm>
#include<xhash>
using namespace std;
class A {
	int a;
	int b;
//	int c;
public:
	A() = default;
	~A() = default;
	A(int _a,int _b):a(_a),b(_b){}
	size_t hashfun() const {
		return hash<int>()(a) << 2 ^ hash<int>()(b);
	}
};
namespace std {
	template<>
	struct hash<A>
	{
		size_t operator()(const A &a)const {
			return a.hashfun();
		}
	};
}
size_t calUOS_reserveSize(size_t need) {
	size_t i = 128;
	while (i < need) i << 1;
	if (i * 0.95 > need) return i;
	else return i << 1;
}
int main() {
/*	cout << hash<A>()(A(5,2)) << endl;
	cout << hash<A>()(A(5, 4)) << endl;
	*/
	unordered_set<int> s;
	/*while (true) {
		int a;
		cin >> a;
	//	calUOS_reserveSize(a);
		s.reserve(calUOS_reserveSize(a));
		cout << s.bucket_count() << endl;
	}*/
	s.insert(20);
	s.insert(10);
	s.insert(10);
	s.size();
	s.erase(18);
	if (s.find(10) == s.end()) cout << "not find" << endl;
	else cout << "find" << endl;
	s.erase(10);
/*
	if (s.find(10) == s.end()) cout << "not find" << endl;
	else cout << "find" << endl;
	cout<<s.bucket(10) << endl;
	s.reserve(100);
	cout << s.size() << endl;
	cout << s.count(10) << endl;
	cout << s.bucket_count() << " " << s.bucket_size(10) << " " << s.max_bucket_count() << endl;
	*/
	return 0;
}

