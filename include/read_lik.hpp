#pragma once

#include <map>
#include <set>
#include <factor_graph.hpp>

typedef std::map<std::string, std::set<std::string>> lik_facs;

lik_facs read_lik(std::string lik_file_path);

std::tuple<FG, FG_Map, FG_Map> read_fg(std::string fg_data);