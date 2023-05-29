#pragma once
#include "predefined.h"
#include <atomic>
#include <graph/edge.h>
#include <mutex>
#include <utility>
#include <variant>

using namespace std;
namespace slf
{
class shared_tasks;
// when the multi_thread is false, the lockfree parameter changes nothing.
template <bool multi_thread, bool lockfree = true>
class tasks
{

    template <typename T>
    using add_atomic_if_multithread =
        std::conditional_t<multi_thread, std::atomic<T>, T>;

    add_atomic_if_multithread<const node_id_t*> begin_{nullptr};
    const node_id_t* end_{nullptr};

public:
    tasks() = default;
    void set_tasks(const node_id_t* _begin, const node_id_t* _end);
    bool get_task(node_id_t& id);
    bool empty() const { return size() == 0; }
    size_t size() const;

    friend class shared_tasks;
};

template <>
class tasks<true, false> : protected tasks<false>
{
    using base_t = tasks<false>;
    std::mutex m_;

public:
    void set_tasks(const node_id_t* _begin, const node_id_t* _end);
    bool get_task(node_id_t& id);
    bool empty() { return size() == 0; }
    size_t size();
};

class shared_tasks : public tasks<true, true>
{
    std::vector<node_id_t> target_match_sequence_;

public:
    const std::vector<node_id_t>& target_match_sequence() const
    {
        return target_match_sequence_;
    }

    bool transfer_tasks(tasks<false>& task,
                        const std::vector<node_id_t>& target_match_sequence);
};

} // namespace slf