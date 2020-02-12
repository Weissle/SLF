#pragma once
#include<algorithm>
#include<vector>
#include<iostream>
#include<string>
#include<mutex>
#include<fstream>
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
	virtual void operator<<(const vector<NodeIDType> &mapping) {	
		if (f.is_open()) {
			put_f(f, mapping);
		}
		put_cout(cout, mapping);
		return;
	}
	void finish() {
		if(f.is_open()) f.close();
	}

};

class AnswerReceiverThread:public AnswerReceiver {
	typedef AnswerReceiver ARBaseType;
	mutex m;
public:
	AnswerReceiverThread() = default;
	AnswerReceiverThread(const std::string &SolutionPath) :ARBaseType(SolutionPath){}
	void operator<<(const vector<NodeIDType>& mapping) {
		lock_guard<mutex> lg(m);
		ARBaseType::operator<<(mapping);
	}
	void finish() {
		ARBaseType::finish();
	}
};