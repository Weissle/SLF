#pragma once
#include "si/shared_task_pool.h"
#include "si/subgraph_isomorphism_base.h"
#include "si/tasks.h"
#include "tools/shared_object_memory_pool.h"
#include <atomic>
#include <memory>
#include <thread>

namespace slf
{
class task_manager
{

    shared_object_memory_pool<shared_tasks> shared_tasks_memory_pool_;
    subgraph_isomorphism_base* solver_;
    atomic_int allow_depth_{0};
    atomic_uint32_t working_thread_;
    uint32_t thread_number_;

    shared_tasks_pool<false> shared_tasks_pool_;

    atomic_size_t shared_tasks_number_{0};
    //  A shared tasks contains many sub-tasks
    atomic_size_t shared_subtasks_number_{0};

    inline void become_idle()
    {
        [[maybe_unused]] uint32_t current_working_thread =
            working_thread_.fetch_sub(1, std::memory_order_relaxed)-1;
        assert(current_working_thread <= thread_number());
        if (current_working_thread == 0 && shared_tasks_pool_.empty())
            stop();
    }

    inline void become_busy()
    {
        [[maybe_unused]] uint32_t current_working_thread =
            working_thread_.fetch_add(1, std::memory_order_relaxed) + 1;
        assert(current_working_thread <= thread_number_);
    }

    inline uint32_t working_thread() const
    {
        return working_thread_.load(std::memory_order_relaxed);
    }

public:
    task_manager(subgraph_isomorphism_base* _solver);
    ~task_manager();
    inline uint32_t thread_number() const { return thread_number_; }

    inline bool end_flag() const { return solver_->is_end(); }
    inline void stop() { solver_->stop(); }

    // For better performance, the try_to_share in paper is equal to
    // allow_sharing_flag + check_share_qualification + share_task
    inline bool allow_sharing_flag()
    {
        return working_thread() != thread_number() &&
               shared_tasks_pool_.empty();
    }

    bool check_share_qualification(size_t depth);
    std::shared_ptr<shared_tasks>
    share_tasks(tasks<false>& task,
               const std::vector<node_id_t>& target_match_sequence);

    bool wait_shared_tasks_or_end(std::shared_ptr<shared_tasks>& ptr_);
};

} // namespace slf
