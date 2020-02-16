#include<iostream>
#include<string>
#include<fstream>
#include<vector>
using namespace std;
class SolutionReader {
public:
	static vector<vector<size_t>> readSolutions(string path) {
		fstream f;
		size_t solutionNum = 0;
		f.open(path.c_str(), ios_base::in);
		if (f.is_open() == false) {
			cout << "open fail" << endl;
			exit(0);
		}
		vector<vector<size_t>> answer;
		while (f.eof() == false) {
			string temp;
			f >> temp;
			if (temp == "solution") {
				int snum, graphsize = 0;
				f >> snum >> graphsize;
				if (answer.size() != snum - 1) cout << "solution num error ?" << endl;
				answer.push_back(vector<size_t>(graphsize));
				LOOP(i, 0, graphsize) {
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