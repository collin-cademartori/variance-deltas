#include <iostream>
#include <fstream>
#include <iterator>
// #include <boost/graph/adjacency_list.hpp>

#include<parameter_graph.hpp>
#include<factor_graph.hpp>

using namespace std;
  
std::pair<MRF, VertexMap> read_mrf(string mrf_file_path) {
  cout << "Reading MRF graph." << endl; 

  VertexMap param_vertices;
  vector<Parameter> params(0);

  // Read MRF file
  ifstream mrf_file(mrf_file_path);
  if (!mrf_file.is_open()) {
      std::cerr << "Could not open mrf file, aborting." << std::endl;
  }

  string mrf_line;
  while(getline(mrf_file, mrf_line) && mrf_line != "---") {
    params.emplace_back(Parameter{ mrf_line });
  }

  vector<pair<int, int>> markov_edges;
  while(getline(mrf_file, mrf_line)){
    auto comma_pos = mrf_line.find(',');
    int e1 = stoi(mrf_line.substr(0, comma_pos)) - 1;
    int e2 = stoi(mrf_line.substr(comma_pos + 1)) - 1;
    if(e1 < e2) {
      markov_edges.emplace_back(make_pair(e1, e2));
    }
  }

  mrf_file.close();

  // Define empty Markov random field
  MRF mrf(0);

  // Add vertices to graph with names stored as internal properties,
  // while also externally mapping names to vertices.
  for_each(params.begin(), params.end(), [&](const Parameter& param) {
    auto vertex = add_vertex(param, mrf);
    param_vertices.emplace(make_pair(param.name, vertex));
  });

  // Add edges to graph
  for_each(markov_edges.begin(), markov_edges.end(), [&](pair<int, int> edge) {
    auto v0 = param_vertices.at(params[edge.first].name);
    auto v1 = param_vertices.at(params[edge.second].name);
    add_edge(v0, v1, mrf);
  });

  cout << "MRF read." << endl;

  return std::make_pair(mrf, param_vertices);
}

std::pair<MRF, VertexMap> mrf_from_fg(FG factor_graph, FG_Map fg_params, FG_Map fg_facs) {
  MRF mrf(0);
  VertexMap MRF_map;

  for(const auto& param: fg_params) {
    Parameter param_data { param.first };
    const auto param_vertex = add_vertex(param_data, mrf);
    MRF_map.emplace(make_pair(param.first, param_vertex));
  }
  
  for(auto& factor: fg_facs) {
    const auto factor_vertex = factor.second;
    const auto param_edges = in_edges(factor_vertex, factor_graph);
    for(auto p1i = param_edges.first; p1i != param_edges.second; ++p1i) {
      const auto p1_name = factor_graph[source(*p1i, factor_graph)].name;
      for(auto p2i = std::next(p1i); p2i != param_edges.second; ++p2i) {
        const auto p2_name = factor_graph[source(*p2i, factor_graph)].name;
        const auto p1_vertex = MRF_map.at(p1_name);
        const auto p2_vertex = MRF_map.at(p2_name);
        add_edge(p1_vertex, p2_vertex, mrf);
      }
    }
  }

  return std::make_pair(mrf, MRF_map);
}