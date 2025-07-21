#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>

#include <markov.hpp>

using namespace std;
using namespace boost;
using namespace markov;

vector<Parameter> params(0);

// Define map from parameter names to vertices.
// Note: This is okay because only edges are mutated, so
// vertex descriptors are not invalidated
map<string, Vertex> param_vertices;

int main(int, char* []) {

  cout << "Reading MRF graph." << endl; 
  // Read MRF file
  ifstream mrf_file("../data/elec_r3.mrf");
  if (!mrf_file.is_open()) {
      std::cerr << "Could not open mrf file, aborting." << std::endl;
      return 1;
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

  // Define empry Markov random field
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

  vertex_names init_names = { "mu_b[49,39]" };
  vertex_names end_names = { "n_democrat_potential[1]" };
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

  MTree test_tree = make_tree(mrf, "mu_b[49,39]", { "n_democrat_potential[1]", "n_democrat_potential[2]" }, {}, param_vertices, 0.9);

  map<Node, size_t> tree_index;
  int vindex = 0;
  const auto tree_nodes = vertices(test_tree);
  for_each(tree_nodes.first, tree_nodes.second, [&tree_index, &vindex](const Node& tree_node){
    tree_index[tree_node] = vindex;
    vindex++;
  });

  ofstream gfile("../graph_test.gv");
  write_graphviz(gfile, test_tree, default_writer{}, default_writer{}, default_writer{}, make_assoc_property_map(tree_index));
  gfile.close();
}