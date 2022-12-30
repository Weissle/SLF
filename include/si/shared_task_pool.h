#pragma once
#include "si/subgraph_isomorphism_base.h"
#include "si/tasks.h"
#include "tools/shared_object_memory_pool.h"
#include <atomic>
#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/queue.hpp>
#include <memory>
#include <mutex>

namespace slf
{
template <bool lockfree>
class shared_tasks_pool;

template <>
class shared_tasks_pool<false>
{
    // also use the using_tasks_mutex ;
    std::mutex shared_tasks_pool_mutex_;
    std::vector<std::shared_ptr<shared_tasks>> pool_;
    void select_heavy_task(std::shared_ptr<shared_tasks>& ptr_)
    {
        int bst_idx = -1;
        float score = -1;
        for (size_t i = 0; i < pool_.size();)
        {
            auto& ptr = pool_[i];
            if (ptr->empty())
            {
                swap(ptr, pool_.back());
                pool_.pop_back();
            }
            else
            {
                int size = ptr->size();
                size_t holding = ptr.use_count();
                assert(holding >= 2);
                float thx_score = size / static_cast<float>(holding - 1);
                if (thx_score > score)
                {
                    score = thx_score;
                    bst_idx = i;
                }
                ++i;
            }
        }
        if (bst_idx != -1)
            ptr_ = pool_[bst_idx];
    }

public:
    shared_tasks_pool(size_t max_len) {
        pool_.reserve(max_len);
    }
    bool get(std::shared_ptr<shared_tasks>& ptr)
    {
        std::lock_guard<std::mutex> lg(shared_tasks_pool_mutex_);
        assert(ptr == nullptr);
        select_heavy_task(ptr);
        return ptr.get();
    }
    bool add(std::shared_ptr<shared_tasks>& ptr)
    {
        std::lock_guard<std::mutex> lg(shared_tasks_pool_mutex_);
        pool_.push_back(ptr);
        return true;
    }
    bool empty()
    {
        std::lock_guard<std::mutex> lg(shared_tasks_pool_mutex_);
        return pool_.empty();
    }
};

template <>
class shared_tasks_pool<true>
{
    // also use the using_tasks_mutex ;
    boost::lockfree::queue<std::shared_ptr<shared_tasks>*,
                           boost::lockfree::fixed_sized<true>>
        queue_;

public:
    shared_tasks_pool(size_t max_len) : queue_(max_len)
    {
        if (queue_.is_lock_free() == false)
            BOOST_LOG_TRIVIAL(error)
                << "shared_tasks_pool<true>::shared_task_pool:"
                   " boost::lockfree::queue is not lockfree";
    }
    bool get(std::shared_ptr<shared_tasks>& ptr)
    {
        assert(ptr == nullptr);
        std::shared_ptr<shared_tasks>* pptr{nullptr};
        while (queue_.pop(pptr))
        {
            if ((*pptr)->empty())
                pptr = nullptr;
            else
            {
                ptr = *pptr;
                add(ptr);
                break;
            }
        }
        return ptr.get();
    }
    bool add(std::shared_ptr<shared_tasks>& ptr)
    {
        return queue_.bounded_push(&ptr);
    }
    bool empty() const { return queue_.empty(); }
};

} // namespace slf
