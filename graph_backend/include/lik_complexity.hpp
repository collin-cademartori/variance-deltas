#pragma once

#include<factor_graph.hpp>

std::function<float(std::set<std::string>)> get_complexity(FG factor_graph, FG_Map fg_params, FG_Map fg_facs);
std::set<std::string> closest_factors(FG_Vertex param_vertex, FG factor_graph);