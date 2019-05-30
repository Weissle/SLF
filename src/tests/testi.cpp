#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<random>
#include<time.h>
using namespace std;
/*
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
class T {
	bool open=false;
	long t = 0;
public:
	T() = default;
	~T() = default;
	void operator()() {
		if (open == false) {
			open = true;
			t = clock();
		}
		else {
			open = false;
			cout << double(clock() - t) / CLOCKS_PER_SEC << endl;
			t = 0;
		}
		return;
	}

};
int main() {
	srand(unsigned int(time(NULL)));
	unordered_set<int> s;
	const int times = 1E8;
	const int count = 1000;
	s.reserve(4086);
	int* p = new int[4086];
	T t;
	cout << s.bucket_count() << endl;
	t();
	for (auto i = 0; i < count; ++i) s.insert(i);
	t();

	t();
	for (auto i = 0; i < times; ++i)s.find(rand() % count);
	t();

	t();
	for (auto i = 0; i < times; ++i) {
		s.erase(rand() % count);
		s.insert(rand() % count);
	}
	t();
	t();
	for (auto i = 0; i < times; ++i)s.find(rand() % count);
	t();
	s.rehash(4096);
	t();
	for (auto i = 0; i < times; ++i)s.find(rand() % count);
	t();
	int a = 0;
	t();
	for (auto i = 0; i < times; ++i)a=p[rand()%count];
	t();


	return 0;
}*/

int main() {
	unordered_map<int, int> m;
	cout << m[1] << endl;
	m[1]++;
	cout << m[1] << endl;
	m[2]++;
	cout << m[2] << endl;
	m[1]++;
	cout << m[1] << endl;


	return 0;
}

