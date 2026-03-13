#pragma once

#include <string>
#include <utility>
#include <factor_graph.hpp>

std::pair<MRF, VertexMap> read_mrf(std::string mrf_file_path);

std::pair<MRF, VertexMap> mrf_from_fg(FG factor_graph, FG_Map fg_params, FG_Map fg_facs);