#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<xhash>
using namespace std;

template<typename L=void>
class A {
	int a;
public:
	A() = default;
	~A() = default;
	A(int _a,L _l):a(_a),l(_l){}
	bool operator==(A a) {
		return l == a.l;
	}
};
int main() {
	
	unordered_set<int> s;
	s.insert(5);
	cout << (s.find(4) == s.end()) << endl;
//	int a = a;
	return 0;
}

