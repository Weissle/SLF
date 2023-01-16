#pragma once
#include "si/results_taker.h"
#include "si/subgraph_isomorphism_base.h"
#include "si/tasks.h"
namespace slf
{

class sequential_subgraph_isomorphism : public subgraph_isomorphism_base
{
    results_taker<false> results_taker_;
    using tasks_t = tasks<false>;
    void search() override;

public:
    sequential_subgraph_isomorphism(std::unique_ptr<graph_t> query,
                                    std::unique_ptr<graph_t> target,
                                    const slf::config::slf_config& slf_config);
    sequential_subgraph_isomorphism(const std::string& query,
                                    const std::string& target,
                                    const slf::config::slf_config& slf_config);
    size_t results_number() const override
    {
        return results_taker_.result_number();
    }
    size_t threads_number() const override { return 1; }
};

} // namespace slf
