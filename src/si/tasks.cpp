#include "si/tasks.h"
namespace slf
{

template <>
void tasks<false>::set_tasks(const node_id_t* _begin, const node_id_t* _end)
{
    begin_ = _begin;
    end_ = _end;
}
template <>
size_t tasks<false>::size() const
{
    assert(end_ >= begin_);
    return end_ - begin_;
}

template <>
bool tasks<false>::get_task(node_id_t& id)
{
    assert(begin_ <= end_);
    if (begin_ == end_)
        return false;
    id = *begin_;
    begin_++;
    return true;
}

template <>
void tasks<true>::set_tasks(const node_id_t* _begin, const node_id_t* _end)
{
    end_ = _end;
    begin_.store(_begin, std::memory_order_release);
}

template <>
size_t tasks<true>::size() const
{
    auto it = begin_.load(std::memory_order_acquire);
    return (end_ > it) ? end_ - it : 0;
}

template <>
bool tasks<true>::get_task(node_id_t& id)
{
    const node_id_t* it;
    it = begin_.fetch_add(1, std::memory_order_acquire);
    if (it >= end_)
        return false;
    id = *it;
    return true;
}

void tasks<true, false>::set_tasks(const node_id_t* _begin,
                                   const node_id_t* _end)
{
    std::lock_guard<std::mutex> lg(m_);
    base_t::set_tasks(_begin, _end);
}

bool tasks<true, false>::get_task(node_id_t& id)
{
    std::lock_guard<std::mutex> lg(m_);
    return base_t::get_task(id);
}

size_t tasks<true, false>::size()
{
    std::lock_guard<std::mutex> lg(m_);
    return base_t::size();
}

bool shared_tasks::transfer_tasks(
    tasks<false>& task, const std::vector<node_id_t>& target_match_sequence)
{
    target_match_sequence_ = target_match_sequence;
    set_tasks(task.begin_, task.end_);
    task.set_tasks(nullptr, nullptr);
    return true;
}

} // namespace slf
