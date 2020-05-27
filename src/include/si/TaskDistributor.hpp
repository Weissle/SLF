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

	mutex free_units_mutex, prepared_units_mutex;
	queue<unique_ptr<SIUnit>> free_units, prepared_units;

	mutex share_tasks_container_mutex, using_tasks_mutex;
	queue<shared_ptr<ShareTasks>> share_tasks_container;
	vector<shared_ptr<ShareTasks>> using_tasks;

	atomic_bool _end;

	bool allow_distribute = false;
	atomic_size_t allow_depth_count;

	//true is left better.
	bool cmp_good_task(const shared_ptr<ShareTasks>& l, const shared_ptr<ShareTasks>& r) const {
		if (l->size() == 0)return false;
		if (l->targetSequence().size() < r->targetSequence().size())return true;
		else return false;
	}
	bool giveTask(unique_ptr<SIUnit> free_unit) {
		bool ok;
		auto tasks = chooseSearchTasks(&ok);
		if (ok == false) {
			lock_guard<mutex> lg_fu(free_units_mutex);
			free_units.push(move(free_unit));
			return false;
		}
		else {
			free_unit->prepare(move(tasks));
			lock_guard<mutex> lg(prepared_units_mutex);
			prepared_units.push(move(free_unit));
			addThreadTask(&TaskDistributor::giveTaskToThreadPool, this);
			return true;
		}
	}
	void giveTaskToThreadPool() {
		prepared_units_mutex.lock();
		auto unit = move(prepared_units.front());
		prepared_units.pop();
		prepared_units_mutex.unlock();
		unit->run();
		addFreeUnit(move(unit));
	}
	void distributeTasks() {
		if (using_tasks.empty())return;
		while (free_units.size()) {
			unique_ptr<SIUnit> free_unit;
			{
				lock_guard<mutex> lg(free_units_mutex);
				if (free_units.empty())	return;
				free_unit = move(free_units.front());
				free_units.pop();
			}
			if (giveTask(move(free_unit)) == false)return;
		}
	}
public:
	TaskDistributor(size_t thread_num_) :ThreadPool(thread_num_) {
		allow_depth_count.store(0);
		_end.store(false);
		using_tasks.reserve(thread_num_ * 2);
		share_tasks_container.push(make_shared<ShareTasks>());
	}
	bool end()const{
		return _end.load();
	}
	void setEnd(const bool end_){
		_end.store(end_);
	}
	void addFreeUnit(unique_ptr<SIUnit> free_unit) {
		lock_guard<mutex> lg(free_units_mutex);
		free_units.push(move(free_unit));
	}
	inline bool allowDistribute() {return allow_distribute;}
	bool haveQuality(const size_t depth){
		if(depth <= allow_depth_count.load() / threadNum()) {
			allow_depth_count.fetch_sub(threadNum());
			return true;
		}
		else{
			allow_depth_count++;
			return false;
		}
	}
	void addShareTasksContainer(shared_ptr<ShareTasks> _c) {
		lock_guard<mutex> lg(share_tasks_container_mutex);
		share_tasks_container.push(_c);
	}
	shared_ptr<ShareTasks> getShareTasksContainer(bool* ok) {
		{
			lock_guard<mutex> lg(using_tasks_mutex);
			for (auto i = 0; i < using_tasks.size(); ++i) {
				if (using_tasks[i].use_count() == 1) {
					*ok = true;
					auto answer = move(using_tasks[i]);
					using_tasks[i] = move(using_tasks.back());
					using_tasks.pop_back();
					return answer;
				}
			}
		}
		assert(share_tasks_container.size() != 0);
		lock_guard<mutex> lg(share_tasks_container_mutex);
		auto answer = share_tasks_container.front();
		share_tasks_container.pop();
		*ok = true;
		return move(answer);
	}
	void addSearchTasks(shared_ptr<ShareTasks> tasks) {
		{
			lock_guard<mutex> lg(using_tasks_mutex);
			using_tasks.push_back(move(tasks));
		}
		allow_distribute = false;
		distributeTasks();
	}
	shared_ptr<ShareTasks> chooseSearchTasks(bool* ok) {
		size_t the_best_task_index = 0;
		lock_guard<mutex> lg(using_tasks_mutex);
		{
			for (auto i = 1; i < using_tasks.size(); ++i) {
				if (using_tasks[i]->size() == 0)continue;
				if (cmp_good_task(using_tasks[the_best_task_index], using_tasks[i]) == false) the_best_task_index = i;
			}
		}
		if (using_tasks.empty() || using_tasks[the_best_task_index]->size() == 0) {
			allow_distribute = true;
			*ok = false;
			return move(make_shared<ShareTasks>());
		}
		else {
			*ok = true;
			return using_tasks[the_best_task_index];
		}
	}


};

}

#endif