#pragma once
#include "tools/ThreadPool.hpp"
#include "si/Tasks.hpp"
#include <memory>
#include <atomic>
#include <mutex>
#include <stack>
#include <condition_variable>


using namespace std;
namespace wg {

template<typename EdgeLabel>
class TaskDistributor {
	using ShareTasksType = ShareTasks<EdgeLabel>;

	mutex using_tasks_mutex;
	vector<shared_ptr<ShareTasksType>> using_tasks;
	
	bool _end = false;

	bool allow_distribute = false;
	size_t allow_depth_count = 0;	//It is used to guess a allow depth, owing to the "guess", we can use size_t instead of atomic_size_t;

	int threadNum;
	mutex workingNumMutex;
	int workingNum;
	
	// also use the using_tasks_mutex ;
	bool task_avaliable = false;
public:
	shared_ptr<condition_variable> wakeupCV;

	TaskDistributor(size_t thread_num_):threadNum(thread_num_){
		using_tasks.reserve(thread_num_ * 3);
		using_tasks.emplace_back(new ShareTasksType());
		wakeupCV = make_shared<condition_variable>();
		//workingNum.store(0);
		workingNum = 0;
	}

	bool end()const {
		return _end;
	}

	void SetOver() {
		_end = true;
		wakeupCV->notify_all();
	}

	inline bool allowDistribute() const{ return !task_avaliable; }
	inline bool taskAvaliable()const{ return task_avaliable; }

	bool haveQuality(const size_t depth) {
		const auto allow_depth = allow_depth_count / threadNum;
		if (depth <= allow_depth) {
			allow_depth_count -= threadNum;
			return true;
		}
		else {
			allow_depth_count += depth - allow_depth;
			return false;
		}
	}

	void PassSharedTasks(shared_ptr<ShareTasksType> tasks) {
		{
			lock_guard<mutex> lg(using_tasks_mutex);
			using_tasks.push_back(move(tasks));
			task_avaliable = true;
		}
		wakeupCV->notify_all();
		//addThreadTask(&TaskDistributor<EdgeLabel,MU>::SharedTasksDistribution, this);
	}

	shared_ptr<ShareTasksType> ChooseHeavySharedTask(bool* ok) {
		static size_t count = 0;
		size_t the_best_task_index = 0;
		lock_guard<mutex> lg(using_tasks_mutex);
		{
			size_t the_best_task_num = 0;
			for (auto i = 1; i < using_tasks.size(); ++i) {
				if (using_tasks[i].use_count() == 1 && using_tasks[i]->size() == 0) {
					swap(using_tasks[i],using_tasks.back());
					using_tasks.pop_back();
					--i;
				}
				else if (the_best_task_num < using_tasks[i]->size()) {
					the_best_task_index = i;
					the_best_task_num = using_tasks[i]->size();
				}
			}
		}
		/*
		{
			cout << count++ << endl;
			for (auto i = 1; i < using_tasks.size(); ++i) {
				cout << using_tasks[i]->size() << " " << using_tasks[i]->depth() << " " << using_tasks[i].use_count() << endl;
			}
		}*/

		if  (using_tasks[the_best_task_index]->empty()) {
			*ok = false;
			task_avaliable = true;
			return nullptr;
		}
		else {
			*ok = true;
			task_avaliable = false;
			return using_tasks[the_best_task_index];
		}
	}

	// It will only happened on beginning and all tasks is finished ( or received limits );
	void ReportBecomeLeisure(){
		lock_guard<mutex> lg(workingNumMutex);
		workingNum--;
		if(workingNum == 0 && task_avaliable == false) SetOver();
	}

	void ReportBecomeBusy(){
		lock_guard<mutex> lg(workingNumMutex);
		workingNum++;
	}


};

}

