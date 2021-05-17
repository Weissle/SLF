#pragma once
#include<algorithm>
#include<vector>
#include<iostream>
#include<string>
#include<mutex>
#include<fstream>
#include<queue>
#include<atomic>
#include<condition_variable>
#include<assert.h>
#include<si/si_marcos.h>
using namespace std;
namespace wg{
inline void printoutSolution(const vector<NodeIDType>& mapping,const size_t &no) {
	cout << "solution " << no << endl;
	for (auto i = 0; i < mapping.size(); ++i) cout << "(" << i << ',' << mapping[i] << ") ";
	cout << endl;
}
class AnswerReceiver {
protected:
	size_t count = 0;
	bool print_solution = false;
public:
	AnswerReceiver() = default;
	AnswerReceiver(bool print_solution_) :print_solution(print_solution_) {}
	void operator<<(const vector<NodeIDType>& mapping) {
		++count;
		if (print_solution) {
			printoutSolution(mapping, count);
		}
		return;
	}

	size_t solutionsCount() { return count; }
};

class AnswerReceiverThread {
	mutex m;
	atomic_size_t atomic_count;
	bool print_solution = false;
public:
	AnswerReceiverThread() { atomic_count.store(0); }
	AnswerReceiverThread(bool print_solution_) :print_solution(print_solution_) { atomic_count.store(0); }
	void operator<<(const vector<NodeIDType>& mapping) {
		atomic_count++;
		if(print_solution){
			lock_guard<mutex> lg(m);
			printoutSolution(mapping, atomic_count.load());
		}
	}
	void solutionCountAdd(size_t s) { 
		atomic_count += s;
	}
	bool printSolution() const { return print_solution; }
	size_t solutionsCount() const { return atomic_count.load(); }

};
}
