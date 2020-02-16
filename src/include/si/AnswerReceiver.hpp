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
	std::fstream f;
	void put_f(fstream& s, const vector<NodeIDType>& mapping) {
		s << "solution " << count << endl;
		s << mapping.size() << endl;
		for (auto i = 0; i < mapping.size(); ++i) {
			s << i << ' ' << mapping[i] << endl;
		}
	}
	void put_cout( ostream & s, const vector<NodeIDType> & mapping){
		s << "solution " << count << endl;
		for (auto i = 0; i < mapping.size(); ++i) cout << "(" << i << ',' << mapping[i] << ") ";
		cout << endl;
	}
public:
	AnswerReceiver() = default;
	AnswerReceiver(const std::string &SolutionPath)
	{
	    f.open(SolutionPath.c_str(), std::ios_base::out);
		if (f.is_open() == false) cout << "solution file open fail" << endl;
	}
	~AnswerReceiver() = default;
	void operator<<(const vector<NodeIDType> &mapping) {	
		if (f.is_open()) {
			put_f(f, mapping);
		}
		put_cout(cout, mapping);
		count++;
		return;
	}
	void finish() {
		if(f.is_open()) f.close();
	}

};
//After subgraph-isomorphism 
class AnswerReceiverThread:public AnswerReceiver {
	mutex m;
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
	AnswerReceiverThread(const std::string &SolutionPath) :AnswerReceiver(SolutionPath){}
	void operator<<(const vector<NodeIDType>& mapping) {
		assert(isFinish == false && "is not finish?");
		lock_guard<mutex> lg(m);
		static size_t c = 0;
		if (c++ % 1000 == 0) {
			cout << " " << c++ << " " << answerQueue.size() * (sizeof(vector<NodeIDType>) + sizeof(NodeIDType) * mapping.size()) << endl;
		}
		answerQueue.push(mapping);
		cv.notify_one();
	}
	void finish() {
		lock_guard<mutex> lg(m);
		isFinish = true;

		outputAll(answerQueue);
		AnswerReceiver::finish();

		cv.notify_one();
	}
	bool empty() {
		lock_guard<mutex> lg(m);
		return answerQueue.empty();
	}
	void run() {
		mutex m1;
		unique_lock<mutex> ul(m1);
		while (true) {
			cv.wait(ul, [&] {return isFinish || answerQueue.empty() == false; });
			if (isFinish) {
				lock_guard<mutex> lg(m);
				outputAll(answerQueue);
				break;
			}
			else {
				m.lock();
				queue<vector<NodeIDType>> newQ;
				swap(newQ, answerQueue);
				m.unlock();
				outputAll(newQ);
			}
		}
		AnswerReceiver::finish();
	}
};