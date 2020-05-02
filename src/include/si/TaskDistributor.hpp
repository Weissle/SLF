#ifndef WG_TASK_DISTRIBUTOR_HPP_
#define WG_TASK_DISTRIBUTOR_HPP_
#include"tools/ThreadPool.hpp"
#include<memory>
#include<atomic>
#include<mutex>
#include<si/si_marcos.h>
#include<queue>
#include"ThreadRelatedClass.hpp"
using namespace std;
namespace wg {

template<class SIUnit>
class TaskDistributor :public ThreadPool {
	void bifurcateFun() {
		distribute_period = true;
		bool ok = false;
		unique_ptr<SIUnit> prepared_unit;
		{
			lock_guard<mutex> lg(prepared_units_mutex);
			prepared_unit = move(prepared_units.front());
			prepared_units.pop();
		}
		prepared_unit->run();
		free_units.push(move(prepared_unit));
	}
	mutex prepared_units_mutex, free_units_mutex;
	queue<unique_ptr<SIUnit>> prepared_units, free_units;
public:
	atomic_bool end = false, distribute_period = false;
	TaskDistributor(size_t thread_num_) :ThreadPool(thread_num_) {}
	void addPreparedUnit(unique_ptr<SIUnit> prepared_unit) {
		{
			lock_guard<mutex> lg(prepared_units_mutex);
			prepared_units.push(move(prepared_unit));
		}
		addTask(&TaskDistributor<SIUnit>::bifurcateFun, this);
	}
	void addFreeUnit(unique_ptr<SIUnit> free_unit) {
		lock_guard<mutex> lg(free_units_mutex);
		free_units.push(move(free_unit));
	}
	unique_ptr<SIUnit> getFreeUnit(bool& ok) {
		lock_guard<mutex> lg(free_units_mutex);
		if (free_units.size()) {
			ok = true;
			auto temp = move(free_units.front());
			assert(&(*temp));
			free_units.pop();
			return move(temp);
		}
		else {
			ok = false;
			return move(unique_ptr<SIUnit>());
		}
	}
	bool allowDistribute() {
		bool answer = (threads_num.load() > running_thread_num.load()) && restTaskNum() == 0 && wait_stop;
//		if (answer)
//			cout << 1 << endl;
		return answer;
	
	}

};

}

#endif