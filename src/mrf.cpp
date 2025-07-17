#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>

#include "min_sep_vis.hpp"

using namespace std;
using namespace boost;

struct Parameter {
  string name;
};

typedef adjacency_list<vecS, vecS, undirectedS, Parameter> MRF;
typedef graph_traits<MRF>::vertex_descriptor Vertex;
typedef graph_traits<MRF>::edge_descriptor Edge;

vector<Parameter> params(0);

// Define map from parameter names to vertices.
// Note: This is okay because only edges are mutated, so
// vertex descriptors are not invalidated
map<string, Vertex> param_vertices;

set<string> minimal_separator_u(MRF mrf, vector<string> u, vector<string> v) {
  add_vertex({ .name = "canary" }, mrf);

  // Find vertices in u by name
  set<Vertex> U;
  for(const string& u_name: u) {
    U.insert(param_vertices[u_name]);
  }

  set<Vertex> NU;
  NU.insert(U.begin(), U.end());
  for(const Vertex& u_vertex: U) {
    auto [adj_begin, adj_end] = adjacent_vertices(u_vertex, mrf);
    for_each(adj_begin, adj_end, [&](const Vertex& adj_vertex) {
      NU.insert(adj_vertex);
    });
  }

  // Find all edges that we need to remove to disconnect neighborhood of U
  // and remove them from the graph, storing the vertex descriptors.
  set<pair<Vertex, Vertex>> disc_edges;
  for(const Vertex& u_vertex: U) {
    auto [adj_begin, adj_end] = adjacent_vertices(u_vertex, mrf);
    for_each(adj_begin, adj_end, [&](const Vertex& adj_vertex) {
      auto [out_begin, out_end] = out_edges(adj_vertex, mrf);
      for_each(out_begin, out_end, [&](const Edge& adj_edge){
        Vertex u1 = source(adj_edge, mrf);
        Vertex u2 = target(adj_edge, mrf);
        if(NU.find(u1) == NU.end() || NU.find(u2) == NU.end()) {
          disc_edges.insert(make_pair(u1, u2));
        }
      }); 
    });
  }

  // Does not invalidate vertex descriptors!
  for(auto& vertex_pair: disc_edges) {
    remove_edge(vertex_pair.first, vertex_pair.second, mrf);
  }

  // Set up map for tracking which vertices are part of the
  // connected component of v, as well as visitor which
  // writes to this map during DFS.
  map<string, bool> conn_v;
  auto [vertex_begin, vertex_end] = vertices(mrf);
  for_each(vertex_begin, vertex_end, [&](const Vertex& vertex){
    conn_v.insert(make_pair(mrf[vertex].name, false));
  });

  component_recorder sep_rec(conn_v);

  // Define external color map to be used by DFS to track visited edges
  map<Edge, default_color_type> dfs_color_map;
  associative_property_map<map<Edge, default_color_type>> dfs_color_property_map(dfs_color_map);

  map<Vertex, default_color_type> dfv_color_map;
  associative_property_map<map<Vertex, default_color_type>> dfv_color_property_map(dfv_color_map);

  // We take the first vertex in v, assuming that all vertices in v
  // are in the same connected component, so that the choice is arbitrary.
  Vertex start_vertex = param_vertices[v[0]];

  cout << "Running DFS" << endl;
  depth_first_visit(mrf, start_vertex, sep_rec, dfv_color_property_map);
  cout << "DFS finished." << endl;

  // For each edge with only one vertex in u component, if other
  // vertex lies in v component, add u vertex to separator.
  // Additionally, repair graph by restoring removed edges.
  set<string> separator;
  for(pair<Vertex, Vertex> vertex_pair: disc_edges) {
    if(conn_v[mrf[vertex_pair.first].name]) {
      separator.insert(mrf[vertex_pair.second].name);
    } else if(conn_v[mrf[vertex_pair.second].name]) {
      separator.insert(mrf[vertex_pair.first].name);
    }
    add_edge(vertex_pair.first, vertex_pair.second, mrf);
  }

  return separator;
}

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

  vector<string> init_names = { "mu_b[49,40]" };
  vector<string> end_names = { "n_democrat_potential[1]" };
  set<string> separator = minimal_separator_u(mrf, end_names, init_names);

  for(auto& sep_name: separator) {
    cout << sep_name << ", ";
  }
  cout << endl << separator.size() << endl;

  set<string> separator_rev = minimal_separator_u(mrf, init_names, end_names);

  for(auto& sep_name: separator_rev) {
    cout << sep_name << ", ";
  }
  cout << endl << separator_rev.size() << endl;
}