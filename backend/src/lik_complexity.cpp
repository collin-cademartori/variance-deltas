#include<lik_complexity.hpp>
#include<factor_graph.hpp>
// #include <iostream>

std::function<float(std::set<std::string>)> get_complexity(FG factor_graph, FG_Map fg_params, FG_Map fg_facs) {

  int num_lik = 0;
  for(const auto& fac: fg_facs) {
    if(factor_graph[fac.second].is_lik) {
      ++num_lik;
    }
  }

  return [factor_graph, fg_params, num_lik](std::set<std::string> params) {
    std::set<std::string> closest_facs {};
    for(const auto& param: params) {
      const auto& param_facs = closest_factors(fg_params.at(param), factor_graph);
      closest_facs.insert(param_facs.begin(), param_facs.end());
    }
    int num_closest = closest_facs.size();

    const float lik_comp = static_cast<float>(num_closest) / static_cast<float>(num_lik);
    return lik_comp;
  };

}

std::set<std::string> closest_factors(FG_Vertex param_vertex, FG factor_graph) {
  bool searching_lik_facs = true;
  std::set<std::string> closest;
  std::set<FG_Vertex> cur_params {param_vertex};
  std::set<FG_Vertex> past_params {param_vertex};
  while(searching_lik_facs) {
    std::set<FG_Vertex> next_params {};

    // Construct set of factors involving current parameters
    std::set<FG_Vertex> cur_facs;
    for(const auto& cur_param_vertex : cur_params) {
       const auto fac_edges = out_edges(cur_param_vertex, factor_graph);
       for(auto fac_edge_it = fac_edges.first; fac_edge_it != fac_edges.second; ++fac_edge_it) {
        cur_facs.insert(target(*fac_edge_it, factor_graph));
       }
      //  cur_facs.insert(fac_edges.first, fac_edges.second);
    }

    // Iterate over current factors, checking if any are likelihood factors
    // If so, keep track of which are likelihood factors
    // If not, construct the set of next parameters to consider
    // std::cout << "Checking factors:" << std::endl;
    if(cur_facs.size() == 0) {
      // std::cout << "ERROR: Empty factors." << std::endl;
      return closest;
    }
    for(auto factor_vertex: cur_facs) {
      // std::cout << factor_graph[factor_vertex].name << "; ";
      if(factor_graph[factor_vertex].is_lik) {
        searching_lik_facs = false;
        closest.emplace(factor_graph[factor_vertex].name);
      } else if (searching_lik_facs) {
        const auto next_param_edges = in_edges(factor_vertex, factor_graph);
        for(auto p_it = next_param_edges.first; p_it != next_param_edges.second; ++p_it) {
          const auto param_next = source(*p_it, factor_graph);
          if(past_params.find(param_next) == past_params.end()) {
            // std::cout << factor_graph[param_next].name << std::endl;
            next_params.emplace(param_next);
          }
        }
      }
      // std::cout << std::endl;
    }

    // Update parameter sets
    past_params.insert(next_params.begin(), next_params.end());
    cur_params.clear();
    cur_params.insert(next_params.begin(), next_params.end());
  }

  return closest;
}