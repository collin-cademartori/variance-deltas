#include <boost/graph/adjacency_list.hpp>

using namespace boost;

typedef std::set<std::string> vertex_names;
typedef std::vector<std::string> vertex_names_v;

struct Parameter {
  std::string name;
};

typedef adjacency_list<vecS, vecS, undirectedS, Parameter> MRF;
typedef graph_traits<MRF>::vertex_descriptor Vertex;
typedef graph_traits<MRF>::edge_descriptor Edge;
typedef std::map<std::string, Vertex> VertexMap;

struct MarkovNode {
  vertex_names parameters;
  std::optional<double> ered;
  int depth;
  std::set<int> chain_nums;
};

typedef adjacency_list<listS, listS, directedS, MarkovNode> MTree;
typedef graph_traits<MTree>::vertex_descriptor Node;
typedef graph_traits<MTree>::edge_descriptor Branch;
