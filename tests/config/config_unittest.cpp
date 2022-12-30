#define BOOST_TEST_MODULE slf_config_unittest
#include "config/slf_config.h"
#include <boost/test/included/unit_test.hpp>
#include <sstream>
namespace slf::config::test
{

BOOST_AUTO_TEST_CASE(slf_config_test)
{
    slf_config cfg;
    std::string json = R"(
		{
			"log":{
				"path":"slf123.log",
				"level":"warning"
			},
			"slf":{
				"thread_number":2,
				"graph_format":"aab",
				"max_log_results": 16,
				"search_results_limitation": 4,
				"search_time_limitation_seconds": 5,
				"tasks":[
					{
						"query":"123.graph",
						"target":"456.graph"
					},
					{
						"query":"890.graph",
						"target":"876.graph"
					}
				]
			}
		}
	)";
    std::stringstream ss(json);
    cfg.read_json_from_stream(ss);
    BOOST_CHECK_EQUAL(cfg.log_level(), "warning");
    BOOST_CHECK_EQUAL(cfg.log_path(), "slf123.log");
    BOOST_CHECK_EQUAL(cfg.graph_format(), "aab");
    BOOST_CHECK_EQUAL(cfg.thread_number(), 2);
    BOOST_CHECK_EQUAL(cfg.max_log_results(), 16);
    BOOST_CHECK_EQUAL(cfg.search_time_limitation_seconds(), 5);
    BOOST_CHECK_EQUAL(cfg.search_results_limitation(), 4);
    cfg.set_log_level("debug");
    cfg.set_log_path("qwe");
    cfg.set_graph_format("cde");
    cfg.set_thread_number(3);
    cfg.set_max_log_results(5);
    cfg.set_search_time_limitation_seconds(6);
    cfg.set_search_results_limitation(5);
    BOOST_CHECK_EQUAL(cfg.log_level(), "debug");
    BOOST_CHECK_EQUAL(cfg.log_path(), "qwe");
    BOOST_CHECK_EQUAL(cfg.graph_format(), "cde");
    BOOST_CHECK_EQUAL(cfg.thread_number(), 3);
    BOOST_CHECK_EQUAL(cfg.max_log_results(), 5);
    BOOST_CHECK_EQUAL(cfg.search_time_limitation_seconds(), 6);
    BOOST_CHECK_EQUAL(cfg.search_results_limitation(), 5);
    auto tasks = cfg.tasks();
    BOOST_CHECK_EQUAL(tasks.size(), 2);
    BOOST_CHECK_EQUAL(tasks[0].query_graph, "123.graph");
    BOOST_CHECK_EQUAL(tasks[1].query_graph, "890.graph");
    BOOST_CHECK_EQUAL(tasks[0].target_graph, "456.graph");
    BOOST_CHECK_EQUAL(tasks[1].target_graph, "876.graph");
}

BOOST_AUTO_TEST_CASE(slf_config_test_default)
{
    slf_config cfg;
    std::string json = R"(
		{
			"slf":{
				"tasks":[]
			}
		}
	)";
    std::stringstream ss(json);
    cfg.read_json_from_stream(ss);
    BOOST_CHECK_EQUAL(cfg.log_level(), "info");
    BOOST_CHECK_EQUAL(cfg.log_path(), "slf.log");
    BOOST_CHECK_EQUAL(cfg.graph_format(), "grf");
    BOOST_CHECK_EQUAL(cfg.thread_number(), 1);
    BOOST_CHECK_EQUAL(cfg.max_log_results(), 0);
    BOOST_CHECK_EQUAL(cfg.search_time_limitation_seconds(), 0);
    BOOST_CHECK_EQUAL(cfg.search_results_limitation(), 0);
}

} // namespace slf::config::test
