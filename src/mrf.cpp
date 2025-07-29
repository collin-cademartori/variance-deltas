#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <nlohmann/json.hpp>

#include <markov.hpp>
#include <ws_client.hpp>
#include <read_mrf.hpp>
#include <read_stan.hpp>
#include <regression.hpp>
#include <serialize_tree.hpp>

using namespace std;
using namespace boost;
using namespace markov;
using json = nlohmann::json;

int main(int, char* []) {

  auto [mrf, param_vertices] = read_mrf("../data/elec_r3.mrf");
  auto stan_data = read_stan_file("../data/standata", 6);

  string leaf_base = "n_democrat_potential";
  set<string> leaves;
  for(int j = 1; j <= 54; ++j) {
    leaves.insert(leaf_base + "[" + to_string(j) + "]");
  }

  auto [mtree, root_node] = make_tree(
    mrf, "mu_b[49,39]", { leaves },
    {}, param_vertices,
    *stan_data.samples, stan_data.vars, 0.9);

  handle_method("get_tree", [&](json _data){
    cout << "Sending tree to server..." << endl;
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

  handle_method("divide_branch", [&root_node, &mtree, &stan_data](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    divide_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

  handle_method("extrude_branch", [&root_node, &mtree, &stan_data](json args) {
    int node_name = args.at("node_name");
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    extrude_branch(*mtree, root_node, node_name, params_kept, *stan_data.samples, stan_data.vars);
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

  handle_method("reset_tree", [&](json args) {
    auto init_tree = make_tree(
      mrf, "mu_b[49,39]", { leaves },
      {}, param_vertices,
      *stan_data.samples, stan_data.vars, 0.9);
    mtree = std::move(init_tree.first);
    root_node = init_tree.second;
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

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