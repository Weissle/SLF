#ifndef WG_TASK_DISTRIBUTOR_HPP_
#define WG_TASK_DISTRIBUTOR_HPP_
#include"tools/ThreadPool.hpp"
#include<memory>
#include<atomic>
#include<mutex>
#include<queue>
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
		addFreeUnit(move(prepared_unit));
	}
	mutex prepared_units_mutex, free_units_mutex;
	queue<unique_ptr<SIUnit>> prepared_units, free_units;

	mutex share_tasks_container_mutex;
	queue<shared_ptr<ShareTasks<NodeIDType>>> share_tasks_container;
public:
	atomic_bool end = false;
	TaskDistributor(size_t thread_num_) :ThreadPool(thread_num_) {
		for (auto i = 0; i < thread_num_; ++i) {
			share_tasks_container.push(make_shared<ShareTasks<NodeIDType>>());
		}
	}
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
//		cout << free_units.size() << endl;
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
		return answer;
	
	}
	void addShareTasksContainer(shared_ptr<ShareTasks<NodeIDType>> _c) {
		lock_guard<mutex> lg(share_tasks_container_mutex);
		share_tasks_container.push(_c);
	}
	shared_ptr<ShareTasks<NodeIDType>> getShareTasksContainer(bool* ok) {
		lock_guard<mutex> lg(share_tasks_container_mutex);
		auto answer = share_tasks_container.front();
		share_tasks_container.pop();
		//push it to the back and it make sure it 
		share_tasks_container.push(answer);
		if (answer->size() != 0 || answer.use_count() != 2) {
			cout << "error 79" << endl;
			*ok = false;
		}
		else *ok = true;
		return answer;
	}
};

}

#endif