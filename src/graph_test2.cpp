#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using namespace std;
using namespace boost;

struct Parameter {
  string name;
};

const vector<Parameter> params = { 
  { .name = "theta" },
  { .name = "mu" },
  { .name = "tau" }
 };

const vector<pair<int, int>> markov_edges = {
  make_pair(0, 1), make_pair(0, 2)
};

int main(int, char* []) {

  // Define empry Markov random field
  typedef adjacency_list<vecS, vecS, undirectedS, Parameter> MRF;
  MRF mrf(0);

  // Define map from parameter names to vertices.
  // Note: This is okay because MRF never mutated after construction,
  // and adding vertices and edges preserves vertex descriptors.
  typedef graph_traits<MRF>::vertex_descriptor Vertex;
  map<string, Vertex> param_vertices;

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

  // Print graph to GraphViz
  ofstream gfile("../graph_test.gv");
  write_graphviz(gfile, mrf);
  gfile.close();
}