#pragma once

#include <set>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<std::set<std::string>> leaves_t;

std::pair<std::string, leaves_t> read_tree_data(std::string tree_data);