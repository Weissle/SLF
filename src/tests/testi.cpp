#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<random>
#include<time.h>
using namespace std;


int main() {
	srand(time(NULL));
	unordered_set<int> m;
	m.reserve(128);
	for (int i = 0; i < 100; ++i) {
		m.insert(rand() % 100);
	}
	for (auto i : m)cout << i << endl;


	return 0;
}

