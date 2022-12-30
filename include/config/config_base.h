#pragma once
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>

namespace slf::config
{
template <typename T>
void get_value(boost::property_tree::ptree ptree_, const char* key, T& value)
{
    auto node = ptree_.get_child_optional(key);
    if (node)
    {
        value = node->get_value<T>();
    }
}

} // namespace slf::config
