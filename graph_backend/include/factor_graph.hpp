#pragma once

#include <boost/graph/adjacency_list.hpp>

typedef std::set<std::string> vertex_names;
typedef std::vector<std::string> vertex_names_v;

struct ModelQuantity {
  std::string name;
  bool is_factor;
  bool is_lik;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & name & is_factor & is_lik;
  }
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ModelQuantity> FG;
typedef boost::graph_traits<FG>::vertex_descriptor FG_Vertex;
typedef boost::graph_traits<FG>::edge_descriptor FG_Edge;
typedef std::map<std::string, FG_Vertex> FG_Map;