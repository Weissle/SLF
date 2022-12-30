#pragma once
#include "common.h"
#include "config/config_base.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>
namespace slf::config
{

class slf_config
{
public:
    struct task
    {
        std::string query_graph, target_graph;
        task(const std::string& q, const std::string& t)
            : query_graph(q), target_graph(t)
        {
        }
    };
    template <typename stream_t>
    bool read_json_from_stream(stream_t& stream)
    {
        boost::property_tree::ptree ptree_;
        boost::property_tree::read_json(stream, ptree_);
        get_value(ptree_, "log.path", log_path_);
        get_value(ptree_, "log.level", log_level_);
        get_value(ptree_, "slf.thread_number", thread_number_);
        get_value(ptree_, "slf.graph_format", graph_format_);
        get_value(ptree_, "slf.max_log_results", max_log_results_);
        get_value(ptree_, "slf.search_time_limitation_seconds",
                  search_time_limitation_seconds_);
        get_value(ptree_, "slf.search_results_limitation",
                  search_results_limitation_);

        const auto& tasks = ptree_.get_child("slf.tasks");
        tasks_.reserve(tasks.size());
        for (const auto& kv : tasks)
        {
            const auto& t = kv.second;
            tasks_.emplace_back(t.get_child("query").get_value<std::string>(),
                                t.get_child("target").get_value<std::string>());
        }

        return true;
    }

    bool read_json(const std::string& path)
    {
        std::ifstream ifstream_;
        if (!open_fstream(path, ifstream_))
            return false;
        return read_json_from_stream(ifstream_);
    }

    const std::string& log_path() const { return log_path_; }
    void set_log_path(const std::string& path) { log_path_ = path; }

    const std::string& log_level() const { return log_level_; }
    void set_log_level(const std::string& level) { log_level_ = level; }

    size_t thread_number() const { return thread_number_; }
    void set_thread_number(size_t tn) { thread_number_ = tn; }

    const std::vector<task>& tasks() const { return tasks_; }
    void tasks(const std::vector<task>& t) { tasks_ = t; }

    const std::string graph_format() const { return graph_format_; }
    void set_graph_format(const std::string& graph_format)
    {
        graph_format_ = graph_format;
    }

    size_t max_log_results() const { return max_log_results_; }
    void set_max_log_results(size_t v) { max_log_results_ = v; }

    long search_time_limitation_seconds() const
    {
        return search_time_limitation_seconds_;
    }
    void set_search_time_limitation_seconds(long val_)
    {
        search_time_limitation_seconds_ = val_;
    }

    size_t search_results_limitation() const
    {
        return search_results_limitation_;
    }
    void set_search_results_limitation(size_t v)
    {
        search_results_limitation_ = v;
    }

private:
    std::string log_path_{"slf.log"};
    std::string log_level_{"info"};
    std::string graph_format_{"grf"};
    size_t max_log_results_{0};
    size_t search_results_limitation_{0};
    long search_time_limitation_seconds_{0};
    std::vector<task> tasks_;
    size_t thread_number_{1};
};
} // namespace slf::config
