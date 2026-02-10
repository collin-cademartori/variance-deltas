#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/uuid.hpp>
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
#include <save_state.hpp>

using namespace std;
using namespace markov;
using json = nlohmann::json;

struct InitState {
  FG fg;
  FG_Map fg_params;
  FG_Map fg_facs;
  std::optional<std::pair<std::unique_ptr<MTree>, Node>> tree;  // populated only for archive
  std::optional<std::string> root_name;   // populated only for files
  std::optional<std::vector<std::set<std::string>>> leaves;  // populated only for files
  std::string sid;
};

InitState init_from_files(const Config& config) {
  auto parser_output = run_model_parser(*config.model_file, *config.data_file);
  if (!parser_output) {
    throw std::runtime_error("Failed to parse model files");
  }
  const auto& [fg_data, tree_data] = *parser_output;

  auto [root_name, leaves] = read_tree_data(tree_data);
  auto [fg, fg_params, fg_facs] = read_fg(fg_data);

  const string& sid = boost::uuids::to_string(boost::uuids::random_generator()());

  return InitState{
    std::move(fg),
    std::move(fg_params),
    std::move(fg_facs),
    std::nullopt,  // tree not yet constructed
    root_name,
    leaves,
    sid
  };
}

InitState init_from_archive(const std::string& archive_path) {
  auto [tree, root_node, fg, fg_params, fg_facs, sid] = load_state(archive_path);

  return InitState{
    std::move(fg),
    std::move(fg_params),
    std::move(fg_facs),
    std::make_pair(std::move(tree), root_node),
    std::nullopt,  // no root_name in archive mode
    std::nullopt,   // no leaves in archive mode
    sid
  };
}

int main(int argc, char* argv[]) {

  #ifndef NDEBUG
    std::cout << "Warning: Running in debug mode, performance may be significantly degraded." << std::endl;
  #endif

  auto result = parse_options(argc, argv);
  if (!result.config) {
    return result.exit_code;
  }
  const Config& config = *result.config;

  // Initialize state from either archive or files
  InitState state;
  try {
    state = config.archive_file
      ? init_from_archive(*config.archive_file)
      : init_from_files(config);
  } catch (const std::exception& e) {
    std::cerr << "Initialization failed: " << e.what() << std::endl;
    return 1;
  }

  // Derive quantities needed for tree construction and method handlers
  const auto likelihood_complexity = get_complexity(state.fg, state.fg_params, state.fg_facs);
  auto [mrf, param_vertices] = mrf_from_fg(state.fg, state.fg_params, state.fg_facs);
  auto stan_data = read_stan_file(config.stan_file_prefix, config.num_chains);

  set<string> global_params = {};
  // Note: global_adj_r needs root_name. In archive mode, get it from the tree.
  string root_name_for_global;
  if (state.root_name) {
    root_name_for_global = *state.root_name;
  } else {
    // In archive mode, extract from tree's root node
    root_name_for_global = to_string(state.tree->first->operator[](state.tree->second).name);
  }
  auto global_adj_r = rf_oob_mse(global_params, root_name_for_global, *stan_data.samples, stan_data.vars);

  // Get or construct tree
  std::unique_ptr<MTree> mtree;
  Node root_node;
  if (state.tree) {
    mtree = std::move(state.tree->first);
    root_node = state.tree->second;
  } else {
    auto [t, r] = make_tree(
      mrf, *state.root_name, { *state.leaves },
      global_params, param_vertices,
      *stan_data.samples, stan_data.vars, likelihood_complexity, 1.01);
    mtree = std::move(t);
    root_node = r;
  }

  handle_method("get_tree", [&](json _data){
    cout << "Sending tree to server..." << endl;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, state.sid));
  });

  handle_method("save_state", [&](json args){
    cout << "Saving backend state to archive." << endl;
    std::string fname = args.at("fname");
    cout << fname << endl;
    try {
      save_state(*mtree, root_node, state.fg, state.fg_params, state.fg_facs, state.sid, fname + ".vds");
    } catch (std::runtime_error e) {
      cerr << "Error while attempting to write archive file: " << e.what() << "\n";
      return("{\"type\":\"io\",\"status\":false}");
    }
    return("{\"type\":\"io\",\"status\":true}");
  });

  handle_method("divide_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    divide_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("auto_divide", [&](json args) {
    int node_name = args.at("node_name");
    auto_divide(*mtree, root_node, node_name, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("extrude_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    extrude_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("delete_node", [&](json args) {
    int node_name = args.at("node_name");
    delete_node(*mtree, root_node, node_name);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("merge_nodes", [&](json args) {
    int node_name = args.at("node_name");
    int alt_node_name = args.at("alt_node_name");
    merge_nodes(mrf, global_params, param_vertices, *mtree, root_node, node_name, alt_node_name, *stan_data.samples, stan_data.vars, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("auto_merge", [&](json args) {
    auto_merge2(mrf, global_params, param_vertices, *mtree, root_node, *stan_data.samples, stan_data.vars, 1, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  handle_method("reset_tree", [&](json args) {
    if (!state.root_name || !state.leaves) {
      std::cerr << "reset_tree is not available when loaded from archive" << std::endl;
      return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
    }
    auto init_tree = make_tree(
      mrf, *state.root_name, *state.leaves,
      global_params, param_vertices,
      *stan_data.samples, stan_data.vars, likelihood_complexity, 1);
    mtree = std::move(init_tree.first);
    root_node = init_tree.second;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, std::nullopt));
  });

  initialize_ws_client("localhost", config.ws_port);
  start_ws_client();

  cout << "WS client start called." << endl;

  return 0;
}
