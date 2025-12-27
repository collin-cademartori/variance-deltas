#include <iostream>
#include <fstream>
#include <iterator>

#include <factor_graph.hpp>
#include <read_lik.hpp>

lik_facs read_lik(std::string lik_file_path) {
  std::cout << "Reading MRF graph." << std::endl; 

  lik_facs factors {};

  // Read LIK file
  std::ifstream lik_file(lik_file_path);
  if (!lik_file.is_open()) {
      std::cerr << "Could not open lik file, aborting." << std::endl;
  }

  bool reading_factor_name = false;
  std::string lik_line;
  while(getline(lik_file, lik_line)) {
    lik_facs::iterator inserted_fac = factors.insert({lik_line, {}}).first;
    std::set<std::string>& fac_params = inserted_fac->second;
    while(getline(lik_file, lik_line) && lik_line != "---") {
      fac_params.insert(lik_line);
    }
  }

  lik_file.close();

  return factors;
}

std::tuple<FG, FG_Map, FG_Map> read_fg(std::string fg_data) {
  std::cout << "Reading factor graph." << std::endl; 

  lik_facs factors {};

  // Read factor graph file
  std::istringstream fg_stream(fg_data);
  // if (!fg_file.is_open()) {
  //     std::cerr << "Could not open factor graph file, aborting." << std::endl;
  // }

  FG factor_graph(0);
  FG_Map fg_params;
  FG_Map fg_factors;
  std::set<std::string> inserted_parameters; 

  std::string fg_line;
  while(getline(fg_stream, fg_line)) {
    
    // Add vertex to factor graph for new factor
    const auto line_len = fg_line.length();
    bool is_lik = fg_line.substr(line_len - 4, std::string::npos) == "_lik";
    const ModelQuantity factor {fg_line, true, is_lik};
    const auto factor_vertex = add_vertex(factor, factor_graph);
    fg_factors.emplace(make_pair(fg_line, factor_vertex));

    // Connect factor node to parameter nodes, adding parameter
    // vertices as necessary.
    while(getline(fg_stream, fg_line) && fg_line != "---") {
      const auto param_node_it = fg_params.find(fg_line);
      FG_Vertex param_vertex;
      if(param_node_it == fg_params.end()) {
        const ModelQuantity param {fg_line, false, false};
        param_vertex = add_vertex(param, factor_graph);
        fg_params.emplace(make_pair(param.name, param_vertex));
      } else {
        param_vertex = param_node_it->second;
      }
      add_edge(param_vertex, factor_vertex, factor_graph);
    }
  }

  // fg_file.close();

  return std::make_tuple(factor_graph, fg_params, fg_factors);
}
