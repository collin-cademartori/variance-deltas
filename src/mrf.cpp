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

vector<Parameter> params(0);

vector<pair<int, int>> markov_edges;

int main(int, char* []) {

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

  while(getline(mrf_file, mrf_line)){
    auto comma_pos = mrf_line.find(',');
    int e1 = stoi(mrf_line.substr(0, comma_pos)) - 1;
    int e2 = stoi(mrf_line.substr(comma_pos + 1)) - 1;
    markov_edges.emplace_back(make_pair(e1, e2));
  }

  mrf_file.close();

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