#pragma once
#include"Pair.hpp"
#include<algorithm>
#include<vector>
#include<iostream>
#include<string>
template<typename _NodeIDType>
class AnswerReceiver {
	typedef _NodeIDType NodeIDType;
	size_t count = 1;
	std::fstream f;
public:
	AnswerReceiver() = default;
	AnswerReceiver(std::string SolutionPath)
	{
	    f.open(SolutionPath.c_str(), std::ios_base::out);
		if (f.is_open() == false) cout << "solution file open fail" << endl;
	}
	~AnswerReceiver() = default;
	void operator<<(const vector<NodeIDType> &mapping) {	
		if (f.is_open()) {
			f << "solution " << count  << endl;
			f << mapping.size() << endl;
			for (auto i = 0; i < mapping.size(); ++i) {
				f << i << ' ' << mapping[i] << endl;
			}
		}
	
		std::cout << "Solution : " << count++ << std::endl;
		for (auto i = 0; i < mapping.size();++i) {

			std::cout << '(' << i << ',' << mapping[i] << ") ";
		}
		std::cout << std::endl;
		return;
	}
	void finish() {
		if(f.is_open()) f.close();
	}

};