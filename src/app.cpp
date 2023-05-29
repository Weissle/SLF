#include "app.h"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
namespace slf
{

std::string parse_argv(int argc, const char* argv[], const char* desc_str)
{

    boost::program_options::options_description desc{desc_str};
    desc.add_options()("help,h", "Show This help information")(
        "config,c", boost::program_options::value<std::string>(),
        "Config (json)");
    boost::program_options::variables_map var_map;
    std::string config_path;
    try
    {
        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, desc),
            var_map);
        boost::program_options::notify(var_map);
        if (var_map.count("config"))
        {
            config_path = var_map["config"].as<std::string>();
        }
        else
        {
            std::cout << desc;
            exit(0);
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Meet error when parse the argument: " << ex.what()
                  << std::endl;
        exit(1);
    }
    return config_path;
}

void initialize_logger(std::string log_path, std::string log_level)
{
    boost::log::add_common_attributes();
    if (log_path.size())
    {
        boost::log::add_file_log(
            boost::log::keywords::file_name = log_path.c_str(),
            boost::log::keywords::format =
                "[%TimeStamp%][%Severity%]: %Message%",
            boost::log::keywords::open_mode = std::ios::out | std::ios::app);
    }

    boost::log::trivial::severity_level level;
    if (log_level == "trace")
        level = boost::log::trivial::severity_level::trace;
    else if (log_level == "debug")
        level = boost::log::trivial::severity_level::debug;
    else if (log_level == "warning")
        level = boost::log::trivial::severity_level::warning;
    else if (log_level == "error")
        level = boost::log::trivial::severity_level::error;
    else if (log_level == "fatal")
        level = boost::log::trivial::severity_level::fatal;
    else // if (log_level == "info")
        level = boost::log::trivial::severity_level::info;
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= level);
}
} // namespace slf
