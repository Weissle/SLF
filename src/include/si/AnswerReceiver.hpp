#pragma once
#include<algorithm>
#include<vector>
#include<iostream>
#include<string>
#include<mutex>
#include<fstream>
#include<queue>
#include<condition_variable>
#include<assert.h>

class AnswerReceiver {
protected:
	typedef size_t NodeIDType;
	size_t count = 1;
	void put_f(fstream& s, const vector<NodeIDType>& mapping) {
		s << "solution " << count << endl;
		s << mapping.size() << endl;
		for (auto i = 0; i < mapping.size(); ++i) {
			s << i << ' ' << mapping[i] << endl;
		}
	}
	void put_cout(ostream& s, const vector<NodeIDType>& mapping) {
		s << "solution " << count << endl;
		for (auto i = 0; i < mapping.size(); ++i) cout << "(" << i << ',' << mapping[i] << ") ";
		cout << endl;
	}
public:
	AnswerReceiver() = default;
	void operator<<(const vector<NodeIDType>& mapping) {
#ifdef PRINT_ALL_SOLUTIONS
		put_cout(cout, mapping);
#endif
		count++;
		return;
	}
	void finish() {
//		cout << "solution count : "<<count-1 << endl;
	}
	size_t solutions_count() { return count - 1; }
};

class AnswerReceiverThread :public AnswerReceiver {
#define OUTPUT_ONCE
	mutex m;
	bool isFinish = false;
public:
	AnswerReceiverThread() = default;
	AnswerReceiverThread(const std::string& SolutionPath) :AnswerReceiver() {}
	void operator<<(const vector<NodeIDType>& mapping) {
		assert(isFinish == false && "is not finish?");
		lock_guard<mutex> lg(m);
		AnswerReceiver::operator<<(mapping);
	}
	void finish() {
		lock_guard<mutex> lg(m);
		isFinish = true;
		AnswerReceiver::finish();
	}

};