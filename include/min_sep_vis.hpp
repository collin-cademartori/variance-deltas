#include <set>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>

using namespace std;
using namespace boost;

class component_recorder : public dfs_visitor<>
{
public:
  component_recorder(map<string, bool>& component_map):
  _component_map(component_map) {}

  template <class Vertex, class Graph> void discover_vertex(Vertex v, const Graph& g) {
    _component_map[g[v].name] = true;
    if(g[v].name == "canary") {
      cout << "TWEET" << endl;
    }
  }

protected:
  map<string, bool>& _component_map;
};
