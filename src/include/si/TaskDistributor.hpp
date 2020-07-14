#ifndef WG_TASK_DISTRIBUTOR_HPP_
#define WG_TASK_DISTRIBUTOR_HPP_
#include"tools/ThreadPool.hpp"
#include"si/Tasks.hpp"
#include<memory>
#include<atomic>
#include<mutex>
#include<queue>
using namespace std;
namespace wg {

template<class SIUnit>
class TaskDistributor :public ThreadPool {
	using EdgeLabelType = typename SIUnit::EdgeLabelType;
	using ShareTasksType = ShareTasks<EdgeLabelType>;
	mutex free_units_mutex, prepared_units_mutex;
	queue<unique_ptr<SIUnit>> free_units, prepared_units;
	mutex share_tasks_container_mutex, using_tasks_mutex;
	queue<shared_ptr<ShareTasksType>> share_tasks_container;
	vector<shared_ptr<ShareTasksType>> using_tasks;

	atomic_bool _end;

	bool allow_distribute = false;
	size_t allow_depth_count = 0;	//It is used to guess a allow depth, owing to the "guess", we can use size_t instead of atomic_size_t;



	void giveTask(unique_ptr<SIUnit> &free_unit, shared_ptr<ShareTasksType> &task) {
		free_unit->prepare(move(task));
		{
			lock_guard<mutex> lg(prepared_units_mutex);
			prepared_units.push(move(free_unit));
		}
		addThreadTask(&TaskDistributor::giveTaskToThreadPool, this);
	}
	void giveTaskToThreadPool() {
		unique_ptr<SIUnit> unit;
		{
			lock_guard<mutex> lg(prepared_units_mutex);
			unit = move(prepared_units.front());
			prepared_units.pop();
		}
		unit->run();
		addFreeUnit(move(unit));
	}
	void distributeTasks() {
		lock_guard<mutex> lg(free_units_mutex);
		allow_distribute = false;
		bool ok;
		shared_ptr<ShareTasksType> task;
		unique_ptr<SIUnit> free_unit;
		while (free_units.size()) {
			free_unit = move(free_units.front());
			free_units.pop();
			task = chooseSearchTasks(&ok);
			if (ok == false)break;
			giveTask(free_unit, task);
		}
		if (free_units.size())allow_distribute = true;
	}
public:
	TaskDistributor(size_t thread_num_) :ThreadPool(thread_num_) {
		//	allow_depth_count.store(0);
		_end.store(false);
		using_tasks.reserve(thread_num_ * 3);
		using_tasks.emplace_back(nullptr);
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
		allow_distribute = true;
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

	shared_ptr<ShareTasksType> getShareTasksContainer() {

		lock_guard<mutex> lg(share_tasks_container_mutex);
		if (share_tasks_container.empty()) {
			return make_shared<ShareTasksType>();
		}
		auto answer = move(share_tasks_container.front());
		share_tasks_container.pop();

		return move(answer);
	}
	void addSearchTasks(shared_ptr<ShareTasksType> tasks) {
		{
			lock_guard<mutex> lg(using_tasks_mutex);
			using_tasks.push_back(move(tasks));
		}
		addThreadTask(&TaskDistributor<SIUnit>::distributeTasks, this);
	}
	shared_ptr<ShareTasksType> chooseSearchTasks(bool* ok) {
		static size_t count = 0;
		size_t the_best_task_index = 0;
		lock_guard<mutex> lg(using_tasks_mutex);
		{
			for (auto i = 1; i < using_tasks.size(); ++i) {
				if (using_tasks[i].use_count() == 1 && using_tasks[i]->size() == 0) {
					lock_guard<mutex> lg2(share_tasks_container_mutex);
					share_tasks_container.push(move(using_tasks[i]));
					using_tasks[i] = move(using_tasks.back());
					using_tasks.pop_back();
					--i;
				}
				else if (using_tasks[i]->size() == 0)continue;
				else if (using_tasks[the_best_task_index].get() == nullptr || using_tasks[the_best_task_index]->size() < using_tasks[i]->size()) the_best_task_index = i;
			}
		}
		/*
		{
			cout << count++ << endl;
			for (auto i = 1; i < using_tasks.size(); ++i) {
				cout << using_tasks[i]->size() << " " << using_tasks[i]->depth() << " " << using_tasks[i].use_count() << endl;
			}
		}*/

		if (the_best_task_index == 0 || using_tasks[the_best_task_index]->empty()) {
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