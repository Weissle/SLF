#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "si/si_marcos.h"


using namespace std;
class SolutionReader {
public:
	static vector<vector<wg::NodeIDType>> readSolutions(string path) {
		fstream f;
		size_t solutionNum = 0;
		f.open(path.c_str(), ios_base::in);
		if (f.is_open() == false) {
			cout << "open fail" << endl;
			exit(0);
		}
		vector<vector<wg::NodeIDType>> answer;
		while (f.eof() == false) {
			string temp;
			f >> temp;
			if (temp == "solution") {
				int snum, graphsize = 0;
				f >> snum >> graphsize;
				if (answer.size() != snum - 1) cout << "solution num error ?" << endl;
				answer.push_back(vector<wg::NodeIDType>(graphsize));
				for (int i = 0; i < graphsize; ++i){ 
					size_t a, b;
					f >> a >> b;
					answer[solutionNum][a] = b;
				}
				solutionNum++;
			}
			else continue;
		}
		return answer;
	}


};
