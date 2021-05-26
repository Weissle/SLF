#pragma once
#include "si/Tasks.hpp"
#include <memory>
#include <atomic>
#include <mutex>
#include <stack>
#include <vector>

using namespace std;
namespace wg {

template<typename EdgeLabel>
class ShareTasksContainerPool{
	using ST = ShareTasks<EdgeLabel>;
	vector<shared_ptr<ST>> using_container,free_container;
	int create_container_num_=0;
	int recycle_threshold;
	mutex m;
	void recycle(){
		int free_num = 0;
		for (int i = 0; i < using_container.size(); ++i){ 
			if(using_container[i].use_count()==1){
				++free_num;
				free_container.emplace_back(using_container[i]);
				swap(using_container[i],using_container.back());
				using_container.pop_back();
				--i;
			}
		}
		if(free_num==0) recycle_threshold++;
	}
public:
	ShareTasksContainerPool(int thread_num_):recycle_threshold(2*thread_num_){
		using_container.reserve(3*thread_num_);
		free_container.reserve(3*thread_num_);
	}
	~ShareTasksContainerPool(){
		//cout<<"create container_num:"<<create_container_num_<<endl;
		//cout<<"recycle_threshold:"<<recycle_threshold<<endl;
	}
	shared_ptr<ST> get(){
		lock_guard<mutex> lg(m);
		if(recycle_threshold <= using_container.size()){
			recycle();
		}
		shared_ptr<ST> ret;
		if(free_container.empty()){
			++create_container_num_;
			ret = make_shared<ST>();
		}
		else{
			ret = move(free_container.back());
			free_container.pop_back();
		}
		using_container.emplace_back(ret);
		return ret;
	}
};

}

