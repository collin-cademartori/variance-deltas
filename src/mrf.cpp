#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>

#include <markov.hpp>
#include <ws_client.hpp>
#include <read_mrf.hpp>

using namespace std;
using namespace boost;
using namespace markov;

// Define map from parameter names to vertices.
// Note: This is okay because only edges are mutated, so
// vertex descriptors are not invalidated

int main(int, char* []) {

  auto [mrf, param_vertices] = read_mrf("../data/elec_r3.mrf");

  MTree test_tree = make_tree(mrf, "mu_b[49,39]", { "n_democrat_potential[1]", "n_democrat_potential[2]" }, {}, param_vertices, 0.9);

  map<Node, size_t> tree_index;
  int vindex = 0;
  const auto tree_nodes = vertices(test_tree);
  for_each(tree_nodes.first, tree_nodes.second, [&tree_index, &vindex](const Node& tree_node){
    tree_index[tree_node] = vindex;
    vindex++;
  });

  ostringstream gstream;
  write_graphviz(gstream, test_tree, default_writer{}, default_writer{}, default_writer{}, make_assoc_property_map(tree_index));
  string graph_str = gstream.str();

  start_ws_client(graph_str);
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