#include <set>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>

class component_recorder : public boost::dfs_visitor<>
{
public:
  component_recorder(std::map<std::string, bool>& component_map):
  _component_map(component_map) {}

  template <class Vertex, class Graph> void discover_vertex(Vertex v, const Graph& g) {
    _component_map[g[v].name] = true;
  }

protected:
  std::map<std::string, bool>& _component_map;
};
