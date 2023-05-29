#include "si/task_manager.h"
#include "si/parallel_subgraph_isomorphism.h"
#include <boost/format.hpp>

namespace slf
{

task_manager::task_manager(subgraph_isomorphism_base* _solver)
    : shared_tasks_memory_pool_(2 * _solver->threads_number()),
      solver_(_solver), thread_number_(_solver->threads_number()),
      shared_tasks_pool_(2 * _solver->threads_number())
{
    working_thread_.store(thread_number_, std::memory_order_relaxed);
}

bool task_manager::wait_shared_tasks_or_end(std::shared_ptr<shared_tasks>& ptr_)
{
    bool report_idle = false;
    ptr_ = nullptr;
    while (true)
    {
        if (end_flag())
            return false;
        else
        {
            shared_tasks_pool_.get(ptr_);
            if (ptr_)
            {
                if (report_idle)
                    become_busy();
                return true;
            }
            else if (!report_idle)
            {
                report_idle = true;
                become_idle();
            }
        }
        sleep_for_us(0);
    }
}

std::shared_ptr<shared_tasks>
task_manager::share_tasks(tasks<false>& task,
                          const std::vector<node_id_t>& target_match_sequence)
{
    auto ptr = shared_tasks_memory_pool_.get();
    shared_tasks_number_.fetch_add(1, std::memory_order_relaxed);
    shared_subtasks_number_.fetch_add(task.size(), std::memory_order_relaxed);
    ptr->transfer_tasks(task, target_match_sequence);
    shared_tasks_pool_.add(ptr);
    return ptr;
}

bool task_manager::check_share_qualification(size_t depth)
{
    if (allow_depth_.fetch_add(1, std::memory_order_relaxed) >=
        static_cast<int>(depth))
    {
        allow_depth_.fetch_sub(thread_number(), std::memory_order_relaxed);
        return true;
    }
    else
        return false;
}

task_manager::~task_manager()
{
    int subtask_left = 0;
    std::shared_ptr<shared_tasks> st;
    while (shared_tasks_pool_.get(st))
    {
        subtask_left += st->size();
        st->set_tasks(nullptr, nullptr);
        st = nullptr;
    }
    BOOST_LOG_TRIVIAL(debug)
        << boost::format("task_manager::~task_manager: shared tasks number: "
                         "[%1%], shared sub-tasks number [%2%], unused shared "
                         "sub-tasks number [%3%]") %
               shared_tasks_number_.load(std::memory_order_relaxed) %
               shared_subtasks_number_.load(std::memory_order_relaxed) %
               subtask_left;
}
} // namespace slf
