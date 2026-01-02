#include <fstream>
#include <iostream>
#include <iterator>
#include <filesystem>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>

#include <lik_complexity.hpp>
#include <markov.hpp>
#include <ws_client.hpp>
#include <read_mrf.hpp>
#include <read_tree_data.hpp>
#include <read_lik.hpp>
#include <read_stan.hpp>
#include <regression.hpp>
#include <serialize_tree.hpp>

namespace options = boost::program_options;
namespace proc = boost::process;
namespace asio = boost::asio;
using namespace std;
using namespace markov;
using json = nlohmann::json;

int main(int argc, char* argv[]) {

  #ifdef NDEBUG
    std::cout << "In release mode." << std::endl;
  #else
    std::cout << "In debug mode." << std::endl; 
  #endif

  options::options_description ops_desc("Command line options.");
  auto ops = ops_desc.add_options();
  ops("help", "print this help message")
  ("model_file,M", options::value<string>()->required(), "specify the model file")
  ("data_file,D", options::value<string>()->required(), "specify the data file")
  ("stan_file_prefix,S",options::value<string>()->required(), "specify the prefix of Stan's MCMC output CSV files")
  ("num_chains,N", options::value<int>()->required(), "specify the number of MCMC chains, i.e. the number of MCMC CSV files to read")
  ("port,P", options::value<int>()->default_value(8765), "specify the WebSocket server port (default: 8765)");

  options::variables_map user_input;

  try {
    options::store(options::parse_command_line(argc, argv, ops_desc), user_input);

    if(user_input.count("help")) {
      std::cout << ops_desc << endl;
      return 0;
    }

    options::notify(user_input);
  } catch (options::required_option& err) {
    std::cerr << "Must specify options " << err.what() << endl << "Run with --help for details." << endl;
    return 1;
  }

  const string& spec_file = user_input["model_file"].as<string>();
  const string& data_file = user_input["data_file"].as<string>();
  const string& stan_file_prefix = user_input["stan_file_prefix"].as<string>();
  const int num_chains = user_input["num_chains"].as<int>();
  const int ws_port = user_input["port"].as<int>();

  // Verify input files exist
  bool files_missing = false;

  if (!std::filesystem::exists(spec_file)) {
    std::cerr << "Error: Model file does not exist: " << spec_file << endl;
    files_missing = true;
  }

  if (!std::filesystem::exists(data_file)) {
    std::cerr << "Error: Data file does not exist: " << data_file << endl;
    files_missing = true;
  }

  // Check if Stan output files exist (check for first chain)
  string first_chain_file = stan_file_prefix + "1.csv";
  if (!std::filesystem::exists(first_chain_file)) {
    std::cerr << "Error: Stan output file does not exist: " << first_chain_file << endl;
    std::cerr << "       (Looking for files with prefix: " << stan_file_prefix << ")" << endl;
    files_missing = true;
  } else {
    // Check if all chains exist
    for (int i = 1; i <= num_chains; ++i) {
      string chain_file = stan_file_prefix + to_string(i) + ".csv";
      if (!std::filesystem::exists(chain_file)) {
        std::cerr << "Error: Stan output file does not exist: " << chain_file << endl;
        files_missing = true;
      }
    }
  }

  if (files_missing) {
    std::cerr << endl << "Please check the file paths and try again." << endl;
    return 1;
  } else {
    std::cout << endl << "All file paths resolved, proceeding." << endl;
  }

  //Polling model code

  // auto [mrf, param_vertices] = read_mrf("../data/election_model/elec_r3.mrf");
  // auto stan_data = read_stan_file("../data/election_model/standata", 6);

  // std::map<std::string, vertex_names> lik_facs {};

  // string leaf_base = "n_democrat_potential";
  // set<string> leaves_set;
  // for(int j = 1; j <= 18; ++j) {
  //   leaves_set.insert(leaf_base + "[" + to_string(j) + "]");
  // }
  // vector<set<string>> leaves;
  // leaves.push_back(leaves_set);

  // string root_name = "mu_b[49,42]"; // day 39, up to 46

  // set<string> global_params = {};

  // Synthetic control model code

  // auto [mrf, param_vertices] = read_mrf("../data/sc_model/sc_model.mrf");
  // auto lik_facs = read_lik("../data/sc_model/sc_model.lik");
  // auto stan_data = read_stan_file("../data/sc_model/sc_reprod_", 4);

  // string leaf_base = "y_pot";
  // vector<set<string>> leaves;
  // for(int j = 1; j <= 6; ++j) { // j <= 6
  //   set<string> leaf;
  //   for(int k = 1; k <= 5; ++k) {
  //     leaf.insert(leaf_base + "[" + to_string(k) + "," + to_string(j) + "]");
  //   }
  //   leaves.push_back(leaf);
  // }

  // set<string> leaf;
  // for(int k = 7; k <= 16; ++k) {
  //   leaf.insert(leaf_base + "_new[" + to_string(k) + ",1]");
  // }
  // leaves.push_back(leaf);

  // string root_name = "causal_effects[4]";

  // set<string> global_params = {
  //   "frac_var_latent", "overall_sd[1]", "overall_sd[2]",
  //   "overall_sd[3]", "overall_sd[4]", "overall_sd[5]", "overall_sd[6]"
  // };

  // Toy hierarchical model code

  // auto [fg, fg_params, fg_facs] = read_fg_from_file("../data/toy_model/toy_model.fg");
  // auto [mrf, param_vertices] = mrf_from_fg(fg, fg_params, fg_facs);
  //auto [mrf, param_vertices] = read_mrf("../data/toy_model/toy_model.mrf");
  // auto lik_facs = read_lik("../data/toy_model/toy_model.lik");
  // auto stan_data = read_stan_file("../data/toy_model/toy_fit", 4);

  // const auto& t1_facs = closest_factors(fg_params.at("theta[1]"), fg);
  // cout << "Closest likelihood factors to theta[1]:" << endl;
  // for(const auto& fac_name : t1_facs) {
  //   cout << fac_name << "; ";
  // }
  // cout << endl;

  // const auto& mu_facs = closest_factors(fg_params.at("mu"), fg);
  // cout << "Closest likelihood factors to mu:" << endl;
  // for(const auto& fac_name : mu_facs) {
  //   cout << fac_name << "; ";
  // }
  // cout << endl;

  // const auto likelihood_complexity = get_complexity(fg, fg_params, fg_facs);
  // const auto c1 = likelihood_complexity({"theta[1]","mu"});
  // const auto c2 = likelihood_complexity({"theta[1]","theta[2]"});
  // std::cout << "Complexity of {theta[1], mu} is: " << c1 << std::endl;
  // std::cout << "Complexity of {theta[1], theta[2]} is: " << c2 << std::endl;

  // string leaf_base = "y_pot";
  // vector<set<string>> leaves;
  // for(int j = 1; j <= 3; ++j) {
  //   set<string> leaf;
  //   for(int k = 1; k <= 2; ++k) {
  //     leaf.insert(leaf_base + "[" + to_string(k) + "," + to_string(j) + "]");
  //   }
  //   leaves.push_back(leaf);
  // }

  // string root_name = "mu";

  // Begin generic algorithm

  asio::io_context ioc;
  asio::readable_pipe interp_pipe{ioc};

  cout << "About to run parser..." << endl;

  proc::process interp_proc(
    ioc,
    "./model_parser", 
    { spec_file, "-d", data_file },
    proc::process_stdio({{}, interp_pipe, {}})
  );

  string interp_data;
  boost::system::error_code pipe_code;
  asio::read(interp_pipe, asio::dynamic_buffer(interp_data), pipe_code);

  if(pipe_code != asio::error::eof) {
    cout << "Error reading interpreter output." << endl;
    return 1;
  }

  interp_proc.wait();

  cout << "Parser ran" << endl;

  const int tree_begin = interp_data.find("\n--");
  const string fg_data = interp_data.substr(0, tree_begin);
  const string tree_data = interp_data.substr(tree_begin + 4);

  set<string> global_params = {};
  // set<string> global_params = {
  //   "frac_var_latent", "overall_sd[1]", "overall_sd[2]",
  //   "overall_sd[3]", "overall_sd[4]", "overall_sd[5]", "overall_sd[6]"
  // };
  auto [root_name, leaves] = read_tree_data(tree_data);  
  auto [fg, fg_params, fg_facs] = read_fg(fg_data);
  const auto likelihood_complexity = get_complexity(fg, fg_params, fg_facs);
  auto [mrf, param_vertices] = mrf_from_fg(fg, fg_params, fg_facs);
  auto stan_data = read_stan_file(stan_file_prefix, num_chains);

  auto global_adj_r = adj_r_squared(global_params, root_name, *stan_data.samples, stan_data.vars);

  std::map<std::string, std::set<std::string>> tree_groups {};

  auto [mtree, root_node] = make_tree(
    mrf, root_name, { leaves },
    global_params, param_vertices,
    *stan_data.samples, stan_data.vars, likelihood_complexity, 1.01); // 0.66

  handle_method("get_tree", [&](json _data){
    cout << "Sending tree to server..." << endl;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("divide_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    divide_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("auto_divide", [&](json args) {
    int node_name = args.at("node_name");
    auto_divide(*mtree, root_node, node_name, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("extrude_branch", [&](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    extrude_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("delete_node", [&](json args) {
    int node_name = args.at("node_name");
    delete_node(*mtree, root_node, node_name);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("merge_nodes", [&](json args) {
    int node_name = args.at("node_name");
    int alt_node_name = args.at("alt_node_name");
    merge_nodes(mrf, global_params, param_vertices, *mtree, root_node, node_name, alt_node_name, *stan_data.samples, stan_data.vars, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("auto_merge", [&](json args) {
    auto_merge2(mrf, global_params, param_vertices, *mtree, root_node, *stan_data.samples, stan_data.vars, 1, likelihood_complexity);
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  handle_method("define_group", [&](json args) {
    cout << "Handling group definition..." << endl;
    std::string group_name = args.at("group_name");
    vector<std::string> node_names = args.at("node_names");
    set<string> node_names_set(node_names.begin(), node_names.end());
    tree_groups.insert(std::make_pair(group_name, node_names_set));
    return std::make_optional(serialize_groups(tree_groups, true));
  });

  handle_method("delete_group", [&](json args) {
    cout << "Handling group deletion..." << endl;
    std::string group_name = args.at("group_name");
    tree_groups.erase(group_name);
    return std::make_optional(serialize_groups(tree_groups, true));
  });

  handle_method("reset_tree", [&](json args) {
    auto init_tree = make_tree(
      mrf, root_name, leaves,
      global_params, param_vertices,
      *stan_data.samples, stan_data.vars, likelihood_complexity, 1);
    mtree = std::move(init_tree.first);
    root_node = init_tree.second;
    return std::make_optional(serialize_tree(root_node, *mtree, global_params, global_adj_r, tree_groups));
  });

  initialize_ws_client("localhost", ws_port);
  start_ws_client();

  cout << "WS client start called." << endl;

  // map<Node, size_t> tree_index;
  // int vindex = 0;
  // const auto tree_nodes = vertices(mtree);
  // for_each(tree_nodes.first, tree_nodes.second, [&tree_index, &vindex](const Node& tree_node){
  //   tree_index[tree_node] = vindex;
  //   vindex++;
  // });

  // ostringstream gstream;
  // write_graphviz(gstream, mtree, default_writer{}, default_writer{}, default_writer{}, make_assoc_property_map(tree_index));
  // string graph_str = gstream.str();
}

  // vertex_names init_names = { "mu_b[49,39]" };
  // vertex_names end_names = { "n_democrat_potential[1]" };
  // set<string> separator = minimal_separator(mrf, init_names, end_names, param_vertices).first;

  // for(auto& sep_name: separator) {
  //   cout << sep_name << ", ";
  // }
  // cout << endl << separator.size() << endl;

  // markov_chain test_chain = make_chain(mrf, init_names, end_names, param_vertices, 0.8);
  // for(const vertex_names& chain_link: test_chain) {
  //   for(const string& v_name: chain_link) {
  //     cout << v_name << "; ";
  //   }
  //   cout << endl << "-------------------------------" << endl;
  // }