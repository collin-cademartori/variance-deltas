#include <boost/graph/adjacency_list.hpp>

typedef std::set<std::string> vertex_names;
typedef std::vector<std::string> vertex_names_v;

struct Parameter {
  std::string name;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Parameter> MRF;
typedef boost::graph_traits<MRF>::vertex_descriptor Vertex;
typedef boost::graph_traits<MRF>::edge_descriptor Edge;
typedef std::map<std::string, Vertex> VertexMap;

struct MarkovNode {
  vertex_names parameters;
  std::optional<double> ered;
  int depth;
  std::set<int> chain_nums;
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS, MarkovNode> MTree;
typedef boost::graph_traits<MTree>::vertex_descriptor Node;
typedef boost::graph_traits<MTree>::edge_descriptor Branch;
