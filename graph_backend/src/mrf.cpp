#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <nlohmann/json.hpp>

#include <lik_complexity.hpp>
#include <markov.hpp>
#include <ws_client.hpp>
#include <read_mrf.hpp>
#include <read_tree_data.hpp>
#include <read_lik.hpp>
#include <read_stan.hpp>
#include <regression.hpp>
#include <regression_rf.hpp>
#include <serialize_tree.hpp>
#include <parse_options.hpp>
#include <run_model_parser.hpp>

using namespace std;
using namespace markov;
using json = nlohmann::json;

int main(int argc, char* argv[]) {

  #ifndef NDEBUG
    std::cout << "Warning: Running in debug mode, performance may be significantly degraded." << std::endl;
  #endif

  auto result = parse_options(argc, argv);
  if (!result.config) {
    return result.exit_code;
  }
  const Config& config = *result.config;

  auto parser_output = run_model_parser(config.model_file, config.data_file);
  if (!parser_output) {
    return 1;
  }
  const auto& [fg_data, tree_data] = *parser_output;

  auto [root_name, leaves] = read_tree_data(tree_data);  
  auto [fg, fg_params, fg_facs] = read_fg(fg_data);

  const auto likelihood_complexity = get_complexity(fg, fg_params, fg_facs);
  auto [mrf, param_vertices] = mrf_from_fg(fg, fg_params, fg_facs);
  auto stan_data = read_stan_file(config.stan_file_prefix, config.num_chains);

  set<string> global_params = {};
  auto global_adj_r = rf_oob_mse(global_params, root_name, *stan_data.samples, stan_data.vars);

  auto [mtree, root_node] = make_tree(
    mrf, root_name, { leaves },
    global_params, param_vertices,
    *stan_data.samples, stan_data.vars, likelihood_complexity, 1.01); // 0.66

  handle_method("get_tree", [&](json _data){
    cout << "Sending tree to server..." << endl;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("divide_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    divide_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("auto_divide", [&](json args) {
    int node_name = args.at("node_name");
    auto_divide(*mtree, root_node, node_name, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("extrude_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    extrude_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("delete_node", [&](json args) {
    int node_name = args.at("node_name");
    delete_node(*mtree, root_node, node_name);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("merge_nodes", [&](json args) {
    int node_name = args.at("node_name");
    int alt_node_name = args.at("alt_node_name");
    merge_nodes(mrf, global_params, param_vertices, *mtree, root_node, node_name, alt_node_name, *stan_data.samples, stan_data.vars, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("auto_merge", [&](json args) {
    auto_merge2(mrf, global_params, param_vertices, *mtree, root_node, *stan_data.samples, stan_data.vars, 1, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  handle_method("reset_tree", [&](json args) {
    auto init_tree = make_tree(
      mrf, root_name, leaves,
      global_params, param_vertices,
      *stan_data.samples, stan_data.vars, likelihood_complexity, 1);
    mtree = std::move(init_tree.first);
    root_node = init_tree.second;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r));
  });

  initialize_ws_client("localhost", config.ws_port);
  start_ws_client();

  cout << "WS client start called." << endl;

  return 0;
}