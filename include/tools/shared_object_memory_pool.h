#pragma once
#include <atomic>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace slf
{
template <typename T>
class shared_object_memory_pool
{
    std::vector<std::shared_ptr<T>> objects_;
    std::unique_ptr<std::atomic_flag[]> flags_;
    std::atomic_size_t index_{0};
    std::atomic_size_t extract_objects_number_{0};

public:
    shared_object_memory_pool(size_t objects_number)
    {
        objects_.resize(objects_number);
        flags_ = std::make_unique<std::atomic_flag[]>(objects_number);
        for (size_t i = 0; i < objects_number; ++i)
        {
            objects_[i] = std::make_shared<T>();
            flags_[i].clear();
        }
    }
    std::shared_ptr<T> get()
    {
        const size_t objects_number = objects_.size();
        for (size_t i = 0; i < objects_number; ++i)
        {
            auto idx = (index_.fetch_add(1, std::memory_order_relaxed)) %
                       objects_number;
            if (false == flags_[idx].test_and_set())
            {
                if (objects_[idx].use_count() == 1)
                {
                    auto ret = objects_[idx];
                    flags_[idx].clear();
                    return ret;
                }
                else
                    flags_[idx].clear();
            }
        }

        extract_objects_number_.fetch_add(1, std::memory_order_relaxed);
        BOOST_LOG_TRIVIAL(warning)
            << "shared_object_memory_pool::get: All objects are being used. An "
               "extract object is created";
        return std::make_shared<T>();
    }
    ~shared_object_memory_pool()
    {
        BOOST_LOG_TRIVIAL(debug)
            << boost::format("shared_object_memory_pool::~shared_object_memory_"
                             "pool: [%1%] "
                             "extract objects are created.") %
                   extract_objects_number_.load(std::memory_order_relaxed);
    }
    // For UT;
    bool all_objects_are_usable()
    {
        for (auto& ptr : objects_)
        {
            if (ptr.use_count() > 1)
                return false;
        }
        return true;
    }
};

} // namespace slf
