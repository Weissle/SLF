#pragma once
#include "config/slf_config.h"
#include "predefined.h"
#include "si/state.h"
#include "tools/timer.h"
#include <atomic>
#include <boost/log/trivial.hpp>
#include <thread>

namespace slf
{

class subgraph_isomorphism_base
{
    bool read_graph(const char* name, std::unique_ptr<graph_t>& graph_ptr,
                    std::string& path);
    template <typename T>
    bool prepare_function_framework(T&& f, const char* name)
    {
        bool ok;
        double t{0};
        {
            scope_timer st(t);
            ok = f();
        }
        if (ok)
        {
            BOOST_LOG_TRIVIAL(debug)
                << boost::format(
                       "subgraph_isomorphism_base::prepare_function_framework: "
                       "[%1%] is done. Time cost [%2%ms].") %
                       name % t;
        }
        else
        {
            BOOST_LOG_TRIVIAL(error)
                << boost::format(
                       "subgraph_isomorphism_base::prepare_function_framework: "
                       "Failed to do [%1%]") %
                       name;
        }
        return ok;
    }

protected:
    bool read_query_graph();
    bool read_target_graph();
    bool preprocess_query_graph();
    bool preprocess_target_graph();
    bool compute_match_sequence();
    bool compute_query_complete_sub_state();
    bool compute_preset_tasks();
    void start_time_consuming_guard();
    virtual void search() = 0;
    size_t search_results_limitation_;
    std::unique_ptr<graph_t> query_graph_;
    std::unique_ptr<graph_t> target_graph_;
    std::shared_ptr<const query_complete_sub_state> query_complete_state_sub_ptr_;
    std::shared_ptr<const preset_tasks> preset_tasks_ptr_;

public:
    subgraph_isomorphism_base() = default;
    subgraph_isomorphism_base(std::unique_ptr<graph_t> query,
                              std::unique_ptr<graph_t> target,
                              long _search_time_limitation_seconds,
                              size_t _results_limit);
    subgraph_isomorphism_base(const std::string& query,
                              const std::string& target,
                              const std::string& graph_format,
                              long _search_time_limitation_seconds,
                              size_t _results_limit);
    virtual ~subgraph_isomorphism_base();

    std::vector<node_id_t> match_sequence_;
    bool is_end() const;
    void stop();
    virtual void run();
    virtual size_t results_number() const = 0;
    virtual size_t threads_number() const = 0;

private:
    std::atomic_bool end_{false};
    std::string query_path_, target_path_, graph_format_;
    long search_time_limitation_s_;
    std::thread time_consuming_guard_;
};

} // namespace slf
