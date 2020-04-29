#pragma once;
#ifndef DYNAMIC_THREAD_POOL_HPP
#define DYNAMIC_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <type_traits>
class ThreadPool {

	std::queue<std::function<void()>> tasks;
	std::mutex tasks_mutex;

	std::vector<std::thread> threads;
	std::atomic_size_t threads_num = 0,running_thread_num = 0;

	std::condition_variable wake_up_cv;
	std::atomic_bool wait_stop = false;

	void run_unit(size_t id) {
		while (true) {
			{

				std::function<void()> task;
				{
					std::unique_lock<std::mutex> ul(tasks_mutex);
					wake_up_cv.wait(ul, [this]() {return tasks.size() != 0 || (running_thread_num == 0 && wait_stop); });
					if (tasks.size()==0) {
						wake_up_cv.notify_one();
						return;
					}
					running_thread_num++;
					task = move(tasks.front());
					tasks.pop();
				}
				task();
				running_thread_num--;
			}
		}
	}
public:
	ThreadPool() = default;
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(size_t threads_num_) :threads_num(threads_num_) {
		threads.resize(threads_num.load());
		for (int i = 0; i < threads_num_; ++i) {
			threads[i] = std::move(std::thread(&ThreadPool::run_unit, this, i));
		}
	}
	template<class R>
	std::future<R> addTask(std::function<R()> f) {
		auto task = std::make_shared<std::packaged_task<R()>>(f);
		std::lock_guard<std::mutex> lg(tasks_mutex);
		tasks.push([task]() {(*task)(); });
		wake_up_cv.notify_one();
		return std::move(task->get_future());
	}
	template<class F, class... Args>
	auto addTask(F&& f, Args&&... args)
	{
		using return_type = typename std::result_of<F(Args...)>::type;
		std::function<return_type()> func = std::function<return_type()>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		return move(addTask(func));
	}
	~ThreadPool() {
		wait_stop = true;
		wake_up_cv.notify_all();
		for (auto& t : threads) {
			if (t.joinable())t.join();
		}
	}

	/*	This function will not stop all thread immediately.
		It will stop a thread  */

	size_t threadNum()const {
		return threads_num.load();
	}
	size_t runningThreadNum()const {
		return running_thread_num;
	}
	size_t restTaskNum()const {
		return tasks.size();
	}
	void join() {
		{
			unique_lock<mutex> ul(tasks_mutex);
			wait_stop = true;
		}
		wake_up_cv.notify_one();
		for (auto& t : threads) {
			if (t.joinable())t.join();
			wake_up_cv.notify_one();
		}

	}
};




#endif