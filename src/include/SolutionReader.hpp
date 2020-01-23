#include<iostream>
#include<string>

using namespace std;
class SolutionReader {
public:
	static vector<vector<int>> readSolutions(string path) {
		fstream f;
		size_t solutionNum = 0;
		f.open(path.c_str(), ios_base::in);
		vector<vector<int>> answer;
		while (f.eof() == false) {
			string temp;
			f >> temp;
			if (temp == "solution") {
				int snum,graphsize=0;
				f >> snum >> graphsize;
				if (answer.size() != snum - 1) cout << "solution num error ?" << endl;
				answer.push_back(vector<int>(graphsize));
				LOOP(i, 0, graphsize) {
					int a, b;
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