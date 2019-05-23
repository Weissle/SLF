#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<xhash>
#include<random>
#include<time.h>
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
//	srand(unsigned int(time(NULL)));
	unordered_set<int> s;

	s.reserve(50);
	cout << s.bucket_count() << endl;
/*	for (int i = 0; i < 50; i++) {
		int temp = rand();
		cout << temp << endl;
		s.insert(temp);
	}
	for (auto it : s) {
		cout << hash<int>()(it)<<"   "<< it << endl;
	}*/
	//	s.insert(5);
//	cout << *s.find(5) << endl;
//	int a = a;
	return 0;
}

