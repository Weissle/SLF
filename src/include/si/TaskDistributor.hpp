#pragma once
#include "si/Tasks.hpp"
#include <memory>
#include <atomic>
#include <mutex>
#include <stack>
#include <condition_variable>
#include <thread>
#include <map>

using namespace std;
namespace wg {

template<typename EdgeLabel>
class TaskDistributor {
	using ShareTasksType = ShareTasks<EdgeLabel>;

	mutex using_tasks_mutex;
	vector<shared_ptr<ShareTasksType>> using_tasks;
	
	bool _end = false;

	size_t allow_depth_count = 0;	//It is used to guess a allow depth, owing to the "guess", we can use size_t instead of atomic_size_t;

	int threadNum__;
	mutex workingNumMutex;
	int workingNum;
	
	// also use the using_tasks_mutex ;
	bool task_avaliable = false;
public:
	shared_ptr<condition_variable> wakeupCV;
	~TaskDistributor(){
		//cout<<"choose heavy tasks times: "<<into_choose_count<<endl;
		//cout<<"really choose heavy tasks times: "<<really_choose_count<<endl;
	}
	TaskDistributor(size_t thread_num_):threadNum__(thread_num_){
		using_tasks.reserve(thread_num_);
		using_tasks.emplace_back(new ShareTasksType());
		wakeupCV = make_shared<condition_variable>();
		//workingNum.store(0);
		workingNum = 0;
	}

	int threadNum()const{
		return threadNum__;
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
		const auto allow_depth = allow_depth_count / threadNum__;
		if (depth <= allow_depth) {
			allow_depth_count -= threadNum__;
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
	//size_t into_choose_count = 0;
	//size_t really_choose_count = 0;

	shared_ptr<ShareTasksType> ChooseHeavySharedTask() {
		//into_choose_count++;
		if(end() || !task_avaliable) return nullptr;
		size_t the_best_task_index = 0;
		lock_guard<mutex> lg(using_tasks_mutex);
		//really_choose_count++;
		{
			size_t the_best_task_num = 0;
			int not_zero_count = 0;
			for (auto i = 1; i < using_tasks.size(); ++i) {
				if (using_tasks[i]->size() == 0) {
					using_tasks[i] = move(using_tasks.back());
					using_tasks.pop_back();
					--i;
				}
				else{
					int temp = using_tasks[i]->size();
					not_zero_count++;
					if (the_best_task_num < temp) {
						the_best_task_index = i;
						the_best_task_num = temp;
					}
				}
			}
			if(not_zero_count==0) task_avaliable = false;
		}
		//cout<<"this time tasks size : "<<using_tasks.size()<<endl;
		if  (using_tasks[the_best_task_index]->empty()) {
			return nullptr;
		}
		else {
			return using_tasks[the_best_task_index];
		}
	}

	void ReportBecomeLeisure(){
		lock_guard<mutex> lg(workingNumMutex);
		workingNum--;
		// It will only happened on this object just be instantiated and all tasks is finished ( or received limits );
		// But this function will not be call until a match unit get a task.
		if(workingNum == 0 && task_avaliable == false) SetOver();
	}

	void ReportBecomeBusy(){
		lock_guard<mutex> lg(workingNumMutex);
		workingNum++;
	}

};

}

