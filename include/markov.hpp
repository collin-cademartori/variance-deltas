#include <parameter_graph.hpp>

using namespace std;

namespace markov {

  typedef list<vertex_names> markov_chain;

  // vertex_names minimal_separator_u(MRF mrf, vertex_names u, vertex_names v, const VertexMap& param_vertices);

  // pair<vertex_names, bool> minimal_separator(MRF mrf, vertex_names u, vertex_names v, const VertexMap& param_vertices);

  markov_chain make_chain(MRF mrf, vertex_names source, vertex_names sink, const map<string, Vertex>& param_vertices, double y_cut);

  MTree make_tree(MRF mrf, const string& root, const vector<string> leaves, const vertex_names& globals, VertexMap& param_vertices, double y_cut);

}