#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <numeric>
#include <Eigen/Dense>

#include <markov.hpp>
#include <min_sep_vis.hpp>
#include <regression.hpp>

using namespace std;
using namespace boost;
using namespace markov;

vertex_names minimal_separator_u(MRF mrf, vertex_names u, vertex_names v, const map<string, Vertex>& param_vertices) {
  // Find vertices in u by name
  set<Vertex> U;
  for(const string& u_name: u) {
    try {
      U.insert(param_vertices.at(u_name));
    } catch (const out_of_range& err) {
      cout << "Error: could find vertex with name " << u_name << ": " << err.what() << endl;
    }
  }

  set<Vertex> NU;
  NU.insert(U.begin(), U.end());
  for(const Vertex& u_vertex: U) {
    auto [adj_begin, adj_end] = adjacent_vertices(u_vertex, mrf);
    for_each(adj_begin, adj_end, [&](const Vertex& adj_vertex) {
      NU.insert(adj_vertex);
    });
  }

  // Check if u and v are separable. If not, return empty set, else proceed.
  bool separable = true;
  for(const string& v_name: v) {
    if(NU.find(param_vertices.at(v_name)) != NU.end()) {
      separable = false;
      break;
    }
  }
  if(!separable) {
    return {};
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
  Vertex start_vertex;
  try {
    start_vertex = param_vertices.at(*v.begin());
  } catch (const std::out_of_range& err) {
    cout << "Could not access element of v set! " << err.what() << endl; 
  }

  depth_first_visit(mrf, start_vertex, sep_rec, dfv_color_property_map);

  // For each edge with only one vertex in u component, if other
  // vertex lies in v component, add u vertex to separator.
  // Additionally, repair graph by restoring removed edges.
  vertex_names separator;
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

pair<vertex_names, bool> minimal_separator(MRF mrf, vertex_names u, vertex_names v, const map<string, Vertex>& param_vertices) {
  vertex_names min_u = minimal_separator_u(mrf, u, v, param_vertices);
  if(min_u.size() < 2) {
    return {min_u, false};
  } else {
    vertex_names min_v = minimal_separator_u(mrf, v, u, param_vertices);
    if(min_u.size() < min_v.size()) {
      return {min_u, false};
    } else {
      return {min_v, true};
    }
  }
}

pair<vertex_names, vertex_names> split_chain(markov_chain& chain, const markov_chain::iterator& pos) {
  vertex_names start {};
  vertex_names end {};
  for_each(chain.begin(), pos, [&start](vertex_names& names) {
    for(const string& name: names) {
      start.insert(name);
    }
  });
  for_each(pos, chain.end(), [&end](vertex_names names) {
    for(const string& name: names) {
      end.insert(name);
    }
  });
  return {start, end};
}

markov_chain markov::make_chain(MRF mrf, vertex_names source, vertex_names sink, const map<string, Vertex>& param_vertices, double y_cut) {

  set<string> y_names;
  for(auto [param_name, param_vertex]: param_vertices) {
    if(param_name.substr(0, 2) == "y_") {
      y_names.insert(param_name);
    }
  }
  int num_ys = y_names.size();

  bool separable = true;
  markov_chain chain = {source};
  auto insert_point = chain.end();
  bool closer_to_v = true;

  vertex_names cur_start = source;
  vertex_names cur_end = sink;
  vertex_names separator = sink;

  while(separable) {

    chain.insert(insert_point, separator);
    if(closer_to_v) {
      insert_point = std::prev(insert_point);
    }

    auto split = split_chain(chain, insert_point);
    cur_start = split.first;
    cur_end = split.second;

    auto separator_data = minimal_separator(mrf, cur_start, cur_end, param_vertices);
    separator = separator_data.first;
    closer_to_v = separator_data.second;

    set<string> y_int;
    set_intersection(y_names.begin(), y_names.end(),
                     separator.begin(), separator.end(),
                     std::inserter(y_int, y_int.begin()));
    double y_perc = static_cast<double>(y_int.size()) / static_cast<double>(num_ys);

    if(separator.size() == 0 || y_perc > y_cut) {
      separable = false;
    }

  }

  return chain;
}

set<int> int_range(int lower, int upper) {
  set<int> irange {};
  for(int i = lower; i < upper; ++i) {
    irange.insert(i);
  }
  return(irange);
}

std::optional<Node> search_children(Node parent_node, vertex_names parameters, const unique_ptr<MTree>& tree) {
  std::optional<Node> found_node = nullopt;
  auto [branch_it, branch_end] = out_edges(parent_node, *tree);
  while(branch_it != branch_end) {
    Node child_node = target(*branch_it, *tree);
    if((*tree)[child_node].parameters == parameters) {
      found_node = child_node;
    }
    branch_it = std::next(branch_it);
  }
  // for_each(branches.first, branches.second, [tree, &parameters, &found_node](const Branch& branch) {
  //   Node child_node = target(branch, *tree);
  //   if((*tree)[child_node].parameters == parameters) {
  //     found_node = child_node;
  //   }
  // });
  return found_node;
}

string print_set(set<string> sset) {
  int nprint = 0;
  string setmsg = "{";
  for(const string& sstr: sset) {
    if (nprint == 2 || nprint == sset.size() - 1) {
      setmsg = setmsg + sstr;
      break;
    } else if(nprint < 2) {
      setmsg = setmsg + sstr + ", ";
    } 
    nprint++;
  }
  setmsg += "}";
  return setmsg;
}

// TBD: Add global params functionablity
std::pair<unique_ptr<MTree>, Node> markov::make_tree(
  MRF mrf, const string& root, const vector<vertex_names> leaves, 
  const vertex_names& globals, 
  VertexMap& param_vertices,
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  double y_cut = 1
) {
  int num_leaves = leaves.size();
  cout << "Number of leaves is " << num_leaves << endl;
  vector<markov_chain> chains(num_leaves);
  vector<markov_chain::iterator> chain_it(num_leaves);
  for(int ci = 0; ci < num_leaves; ++ci) {
    chains[ci] = markov::make_chain(mrf, { root }, leaves[ci], param_vertices, y_cut);
    chain_it[ci] = chains[ci].begin();
  }

  stack<Node> node_stack;
  unique_ptr<MTree> markov_tree = make_unique<MTree>(0);

  auto hf = std::hash<string> {};

  vertex_names params = { root };
  auto name_hash = hf(root);
  Node root_node = add_vertex({ 
    .name = name_hash,
    .parameters = params,
    .ered = 0,
    .depth = 0,
    .chain_nums = int_range(0, num_leaves)
  }, *markov_tree);
  node_stack.push(root_node);


  while(node_stack.size() > 0) {
    Node cur_node = node_stack.top();
    node_stack.pop();
    set<int> cur_node_chains((*markov_tree)[cur_node].chain_nums);
    for(int ci: cur_node_chains) {
      //cout << "CI is " << ci << endl;
      int cur_depth = (*markov_tree)[cur_node].depth;
      if(std::next(chain_it[ci]) != chains[ci].end()) {
        chain_it[ci] = std::next(chain_it[ci]);
        vertex_names chain_parameters = *chain_it[ci];

        std::optional<Node> next_node = search_children(cur_node, chain_parameters, markov_tree);
        if(next_node == nullopt) {
          double ered = sqrt(adj_r_squared(chain_parameters, root, stan_matrix, stan_vars));
          auto name_hash = hf(std::reduce<vertex_names::iterator, string>(chain_parameters.begin(), chain_parameters.end(), ""));
          Node new_node = add_vertex({ 
            .name = name_hash,
            .parameters = chain_parameters,
            .ered = ered,
            .depth = cur_depth + 1,
            .chain_nums = { ci }
          }, *markov_tree);
          cout << "Connecting " << print_set((*markov_tree)[cur_node].parameters) << " to " << print_set((*markov_tree)[new_node].parameters) << "." << endl;
          add_edge(cur_node, new_node, *markov_tree);
          node_stack.push(new_node);
        } else {
          cout << "Found child!" << endl;
          (*markov_tree)[next_node.value()].chain_nums.insert(ci);
        }
      }
    }
  }

  return(std::make_pair(std::move(markov_tree), root_node));
}

void markov::divide_branch(
  MTree& tree, const Node& root, 
  size_t node_name, vertex_names params_kept, 
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars
) {
  cout << "Beginning divide branch..." << endl;
  auto hf = std::hash<string> {};

  std::queue<Node> node_queue {};
  node_queue.push(root);
  bool not_found = true;
  std::optional<std::pair<Node, Node>> split_nodes = std::nullopt;
  while(node_queue.size() > 0 && not_found) {
    Node cur_node = node_queue.front();
    node_queue.pop();

    auto out_it = out_edges(cur_node, tree);
    for_each(out_it.first, out_it.second, [&](Branch branch) {
      Node child_node = target(branch, tree);
      if(tree[child_node].name == node_name) {
        not_found = false;
        split_nodes = std::make_pair(cur_node, child_node);
      } else if(not_found) {
        node_queue.push(child_node);
      }
    });
  }
  cout << "Found child node!" << endl;

  if(split_nodes == nullopt) {
    throw new std::out_of_range("Could not locate child node! Cannot divide branch.");
  } else {
    cout << "Modifying tree..." << endl;
    auto [par_node, child_node] = split_nodes.value();
    remove_edge(par_node, child_node, tree);

    auto child_params = tree[child_node].parameters;
    params_kept.insert(child_params.begin(), child_params.end());
    string root_name = *tree[root].parameters.begin();
    double ered = sqrt(adj_r_squared(params_kept, root_name, stan_matrix, stan_vars));
    auto name_hash = hf(std::reduce<vertex_names::iterator, string>(params_kept.begin(), params_kept.end(), ""));
    Node split_node = add_vertex({ 
      .name = name_hash,
      .parameters = params_kept,
      .ered = ered,
      .depth = tree[par_node].depth + 1,
      .chain_nums = { }
    }, tree);

    add_edge(par_node, split_node, tree);
    add_edge(split_node, child_node, tree);

    // Fix depth?
  }
}