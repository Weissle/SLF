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
	size_t allow_depth_count = 0;	//It is used to guess a allow depth, owing to the "guess", we can use size_t instead of atomic_size_t;



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
		//	allow_depth_count.store(0);
		_end.store(false);
		using_tasks.reserve(thread_num_ * 3);
		using_tasks.emplace_back(nullptr);
		share_tasks_container.push(make_shared<ShareTasks>());
	}
	void output_hittimes() {
		while (free_units.size()) {
			unique_ptr<SIUnit> free_unit;
			{
				lock_guard<mutex> lg(free_units_mutex);
				if (free_units.empty())	return;
				free_unit = move(free_units.front());
				free_units.pop();
			}
			cout << free_unit->run_clock << endl;
		}
	}
	bool end()const {
		return _end.load();
	}
	void setEnd(const bool end_) {
		_end.store(end_);
	}
	void addFreeUnit(unique_ptr<SIUnit> free_unit) {
		lock_guard<mutex> lg(free_units_mutex);
		free_units.push(move(free_unit));
	}
	inline bool allowDistribute() { return allow_distribute; }
	bool haveQuality(const size_t depth) {
		const auto allow_depth = allow_depth_count / threadNum();
		if (depth <= allow_depth) {
			allow_depth_count -= threadNum();
			return true;
		}
		else {
			allow_depth_count += depth - allow_depth;
			return false;
		}
	}

	shared_ptr<ShareTasks> getShareTasksContainer() {

		lock_guard<mutex> lg(share_tasks_container_mutex);
		if (share_tasks_container.empty()) {
			return make_shared<ShareTasks>();
		}
		auto answer = move(share_tasks_container.front());
		share_tasks_container.pop();

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
		static size_t count = 0;
		size_t the_best_task_index = 0;
		lock_guard<mutex> lg(using_tasks_mutex);
		{
			for (auto i = 1; i < using_tasks.size(); ++i) {
				if (using_tasks[i].use_count() == 1) {
					swap(using_tasks[i], using_tasks.back());
					lock_guard<mutex> lg2(share_tasks_container_mutex);
					share_tasks_container.push(move(using_tasks.back()));
					using_tasks.pop_back();
					--i;
				}
				else if (using_tasks[i]->size() == 0)continue;
				else if (using_tasks[the_best_task_index].get() == nullptr || using_tasks[the_best_task_index]->size() < using_tasks[i]->size()) the_best_task_index = i;
			}
		}

		if (the_best_task_index == 0 || using_tasks[the_best_task_index]->empty()) {
			allow_distribute = true;
			*ok = false;
			return nullptr;
		}
		else {
			*ok = true;
			return using_tasks[the_best_task_index];
		}
	}


};

}

#endif