#include "tools/timer.h"
#include <cassert>
#include <iostream>

namespace slf
{
scope_timer::scope_timer(double& val) : time_duration_(val)
{
    start_ = std::chrono::steady_clock::now();
}

scope_timer::~scope_timer()
{
    time_duration_ = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now() - start_)
                         .count() /
                     1000.0;
}
timer::timer() { reset_start(); }

timer::time_point_t timer::get_current_time_point(bool save)
{
    auto tp = std::chrono::steady_clock::now();
    if (save)
        time_points[last_tag] = tp;
    return tp;
}

void timer::reset_start()
{
    time_points.clear();
    time_points[start_tag] = get_current_time_point(true);
}

void timer::tick(const std::string& tag)
{
    assert(tag != last_tag && tag != start_tag);
    time_points[tag] = get_current_time_point(true);
}

void timer::tick() { get_current_time_point(true); }

timer::milliseconds_t timer::time_duration(const std::string& from,
                                           const std::string& to)
{
    assert(time_points.count(from) && time_points.count(to));
    return std::chrono::duration_cast<milliseconds_t>(time_points[to] -
                                                      time_points[from]);
}

timer::milliseconds_t timer::time_duration_from_last_tick()
{
    return time_duration_from_tag(last_tag);
}
timer::milliseconds_t timer::time_duration_from_start()
{
    return time_duration_from_tag(start_tag);
}
timer::milliseconds_t timer::time_duration_from_tag(const std::string& tag)
{
    assert(time_points.count(tag));
    return std::chrono::duration_cast<milliseconds_t>(
        get_current_time_point(false) - time_points[tag]);
}
} // namespace slf
