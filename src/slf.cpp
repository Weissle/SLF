#include "app.h"
#include "common.h"
#include "config/slf_config.h"
#include "si/parallel_subgraph_isomorphism.h"
#include "si/sequential_subgraph_isomorphism.h"
#include <boost/log/trivial.hpp>
#include <fstream>
#include <iostream>

int main(int argc, const char* argv[])
{
    slf::config::slf_config slf_config_;
    std::string config_path = slf::parse_argv(argc, argv, "SLF");
    if (!slf_config_.read_json(config_path))
    {
        std::cerr << "Fail to open config " << config_path << std::endl;
        exit(1);
    }
    slf::initialize_logger(slf_config_.log_path(), slf_config_.log_level());
    BOOST_LOG_TRIVIAL(info)
        << boost::format("main: log path [%1%], log level [%2%].") %
               slf_config_.log_path() % slf_config_.log_level();

    auto thread_number = slf_config_.thread_number();
    for (const auto& task : slf_config_.tasks())
    {
        std::unique_ptr<slf::subgraph_isomorphism_base> ptr;
        if (thread_number > 1)
            ptr = std::make_unique<slf::parallel_subgraph_isomorphism>(
                task.query_graph, task.target_graph, slf_config_);
        else
            ptr = std::make_unique<slf::sequential_subgraph_isomorphism>(
                task.query_graph, task.target_graph, slf_config_);
        ptr->run();
        boost::log::core::get()->flush();
    }
    return 0;
}
