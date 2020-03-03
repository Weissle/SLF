#pragma once
#include<functional>
#include<vector>
#include<time.h>
#include<assert.h>
using namespace std;
//A test class. It may not work well.
//state change has two ways, one is assignment,the other is iterating from initial state
//assumping the time of assignment cost  is constant or changeless
//and the time of FluctuatingFunc cost usually depend on the number of steps
//this class can choose a better(cost less time) function to create the state automatically;
//template<class StableFunc,class FluctuatingFunc>
class AdaptiveStateChanger {
	vector<size_t> fluTime;//add or delete
	size_t maxDepth;
	size_t stableTime = 0;
	size_t equalPoint = 0;
	size_t guessEqualPoint = 0;
public:
	AdaptiveStateChanger() = default;
	AdaptiveStateChanger(size_t _maxDepth) :maxDepth(_maxDepth + 1) { fluTime.resize(_maxDepth + 1); }

	void stateTurn(function<void(size_t)> &sf,function<void(size_t)>&ff,size_t depth) {
		auto& tempTime = fluTime[depth];
		assert(depth < maxDepth );
		auto useSF = [&]() {
			auto t1 = clock();
			sf(depth);
			stableTime = clock() - t1;
		};
		auto useFF = [&]() {
			auto t1 = clock();
			ff(depth);
			tempTime = clock() - t1;
			if (tempTime == stableTime) equalPoint = depth;
			else {
				double temp =depth * (double)(stableTime) / tempTime;
				guessEqualPoint = (guessEqualPoint == 0) ? temp : (temp * 0.8 + (double)guessEqualPoint * 0.2);
			}
		};
		if (stableTime == 0 || stableTime <= tempTime)	useSF();
		else if (tempTime != 0)	useFF();
		else if (guessEqualPoint) {
			if (guessEqualPoint >= depth)	useFF();
			else useSF();
		}
		else useFF();

		return;
	}
};