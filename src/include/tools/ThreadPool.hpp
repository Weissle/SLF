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
	std::condition_variable wake_up_cv;
protected:
	std::atomic_bool wait_stop;
	std::atomic_size_t threads_num, running_thread_num;
private:
	void run_unit(size_t id) {
		running_thread_num++;
		while (true) {
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> ul(tasks_mutex);
					running_thread_num--;
					wake_up_cv.wait(ul, [this]() {return tasks.size() != 0 || (running_thread_num == 0 && wait_stop); });
					if (running_thread_num == 0 && wait_stop) 	{
						wake_up_cv.notify_all();
						return;
					}
					
					running_thread_num++;
					task = move(tasks.front());
					tasks.pop();
				}
				task();
			}
		}
	}
public:
	ThreadPool() = default;
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(size_t threads_num_) {
		threads_num.store(threads_num_);
		wait_stop.store(false);
		running_thread_num.store(0);
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
	auto addThreadTask(F&& f, Args&&... args)
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
		}

	}
};




#endif