#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<xhash>
using namespace std;

template<typename L=void>
class A {
	L l;
public:
	A() = default;
	~A() = default;
	A(L _l):l(_l){}
	bool operator==(A a) {
		return l == a.l;
	}
};
int main() {
	A<> a1, a2;
	cout << (a1 == a2) << endl;
	return 0;
}

