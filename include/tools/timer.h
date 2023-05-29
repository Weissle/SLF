#pragma once
#include <chrono>
#include <unordered_map>
#include <string>
namespace slf
{

class scope_timer
{
    using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
    time_point_t start_;
    double  &time_duration_;

public:
    scope_timer(double& val);
    ~scope_timer();
};

// TODO: ut
class timer
{
    using milliseconds_t = std::chrono::milliseconds;
    using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
    std::unordered_map<std::string, time_point_t> time_points;
    time_point_t get_current_time_point(bool save);

public:
    inline static const std::string last_tag{"__timer_last"};
    inline static const std::string start_tag{"__timer_start"};
    timer();
    void reset_start();
    void tick(const std::string& tag);
    void tick();
    milliseconds_t time_duration(const std::string& from,
                                 const std::string& to);
    milliseconds_t time_duration_from_last_tick();
    milliseconds_t time_duration_from_start();
    milliseconds_t time_duration_from_tag(const std::string& tag);
};
} // namespace slf
