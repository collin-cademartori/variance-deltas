#pragma once

#include "parameter_graph.hpp"
#include "factor_graph.hpp"
#include <memory>
#include <string>
#include <tuple>
#include <utility>

void save_tree(const MTree& tree, Node root, const std::string& filename);
std::pair<std::unique_ptr<MTree>, Node> load_tree(const std::string& filename);

void save_fg(const FG& fg, const FG_Map& fg_params, const FG_Map& fg_factors, const std::string& filename);
std::tuple<FG, FG_Map, FG_Map> load_fg(const std::string& filename);

void save_state(const MTree& tree, Node root,
                const FG& fg, const FG_Map& fg_params, const FG_Map& fg_factors,
                const std::string& sid,
                const std::string& filename);
std::tuple<std::unique_ptr<MTree>, Node, FG, FG_Map, FG_Map, std::string> load_state(const std::string& filename);
