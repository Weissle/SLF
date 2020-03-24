#ifndef WG_THREAD_CONTROLLER_HPP_
#define WG_THREAD_CONTROLLER_HPP_
#include"tools/ThreadPool.hpp"
#include<memory>
#include<atomic>
#include<mutex>
#include<si/si_marcos.h>
#include"ThreadRelatedClass.hpp"
using namespace std;
namespace wg {

template<class SIUnit>
class ThreadController :public ThreadPool {
	void bifurcateFun() {
		bool ok = false;
		auto prepared_unit = move(prepared_units.pop(ok));
		prepared_unit->run();
		free_units.push(move(prepared_unit));
	}
	stack_mutex<unique_ptr<SIUnit>> prepared_units;
public:
	stack_mutex<unique_ptr<SIUnit>> free_units;
	atomic_bool end = false, distribute_period = false;
	ThreadController(size_t thread_num_) :ThreadPool(thread_num_) {}
	void bifurcate(unique_ptr<SIUnit> prepared_unit) {
		prepared_units.push(move(prepared_unit));
		addTask(&ThreadController<SIUnit>::bifurcateFun, this);
	}

};

}

#endif