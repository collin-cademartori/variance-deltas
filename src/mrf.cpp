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
  
  cout << *(*mtree)[root_node].parameters.begin() << endl;

  handle_method("get_tree", [&root_node, &mtree](json _data){
    cout << "Sending tree to server..." << endl;
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

  auto& stan_matrix = stan_data.samples;

  handle_method("divide_branch", [&root_node, &mtree, &stan_matrix](json args) {
    cout << "Dividing branch..."  << endl;
    cout << args.dump() << endl;
    size_t node_name = args.at("node_name");
    cout << "Read in name!" << endl;
    set<string> params_kept;
    for(const string& param: args.at("params_kept")) {
      params_kept.insert(param);
    }
    cout << "Data prepared!" << endl;
    //divide_branch(*mtree, root_node, node_name, params_kept, *stan_matrix_ptr, stan_vars);
    return std::make_optional(serialize_tree(root_node, *mtree));
  });

  start_ws_client();

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