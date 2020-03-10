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
//	std::fstream f;
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
	AnswerReceiver(const std::string& SolutionPath)
	{
	/*	f.open(SolutionPath.c_str(), std::ios_base::out);
		if (f.is_open() == false) cout << "solution file open fail" << endl;*/
	}
	~AnswerReceiver() = default;
	void operator<<(const vector<NodeIDType>& mapping) {
	/*	if (f.is_open()) {
			put_f(f, mapping);
		}
		put_cout(cout, mapping);*/
		count++;
		return;
	}
	void finish() {
		cout << "solution count : "<<count-1 << endl;
	//	if (f.is_open()) f.close();
	}

};

class AnswerReceiverThread :public AnswerReceiver {
#define OUTPUT_ONCE
	mutex m, output;
	size_t has = 0;
	queue<vector<NodeIDType>> answerQueue;
	bool isFinish = false;
	condition_variable cv;
	void outputAll(queue<vector<NodeIDType>>& q) {
		while (q.empty() == false) {
			AnswerReceiver::operator<<(q.front());
			q.pop();
		}
	}
public:
	AnswerReceiverThread() = default;
	AnswerReceiverThread(const std::string& SolutionPath) :AnswerReceiver(SolutionPath) {}
	void operator<<(const vector<NodeIDType>& mapping) {
		assert(isFinish == false && "is not finish?");
		lock_guard<mutex> lg(m);
		answerQueue.push(mapping);
	}
	void finish() {
		lock_guard<mutex> lg(m);
		isFinish = true;
		cout << answerQueue.size() << endl;
	//	AnswerReceiver::finish();
	}
	bool empty() {
		lock_guard<mutex> lg(m);
		return answerQueue.empty();
	}
};