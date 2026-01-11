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

// STOP GAP SOLUTION
int max_id = 0;
map<string, int> id_map;
int get_id(string set_name) {
  int id;
  try {
    id = id_map.at(set_name);
  } catch (out_of_range err) {
    id = ++max_id;
  }
  return id;
}

set<string> set_minus(set<string> pset, set<string> globals) {
  // for (auto pset_it = pset.begin(); pset_it != pset.end(); std::next(pset_it)){
  //   if(std::find(globals.begin(), globals.end(), *pset_it) != globals.end()) {
  //     pset.erase(pset_it);
  //   }
  // }
  for(const string& global_param: globals) {
    pset.erase(global_param);
  }
  return(pset);
}

void write_set(set<string>& pset) {
  cout << "{";
  for(auto pset_it = pset.begin(); pset_it != pset.end(); pset_it = std::next(pset_it)) {
    cout << *pset_it;
    if(std::next(pset_it) != pset.end()) {
      cout << "; ";
    }
  }
  cout << "}";
}

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
    try {
      if(NU.find(param_vertices.at(v_name)) != NU.end()) {
        separable = false;
        break;
      }
    } catch (std::out_of_range _err) {
      cout << "Could not locate vertex " << v_name << "!" << endl;
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

pair<vertex_names, bool> minimal_separator(MRF mrf, vertex_names u, vertex_names v, const map<string, Vertex>& param_vertices,
                                           std::function<float(std::set<std::string>)> LC) {
  vertex_names min_u = minimal_separator_u(mrf, u, v, param_vertices);
  if(min_u.size() < 2) {
    return {min_u, false};
  } else {
    vertex_names min_v = minimal_separator_u(mrf, v, u, param_vertices);

    float u_complexity = LC(min_u);
    float v_complexity = LC(min_v);

    cout << "Separators found:" << endl;
    write_set(min_u);
    cout << "Complexity: " << to_string(u_complexity) << endl;
    cout << endl;
    write_set(min_v);
    cout << "Complexity: " << to_string(v_complexity) << endl;
    cout << endl << endl;

    // int u_complexity = 0;
    // int v_complexity = 0;
    // for (const auto& fac : lik_facs) {
    //   const std::set<std::string>& fac_set = fac.second;
    //   std::set<std::string> fac_int_u {};
    //   std::set<std::string> fac_int_v {};
    //   std::set_intersection(
    //     min_u.begin(), min_u.end(),
    //     fac_set.begin(), fac_set.end(),
    //     std::inserter(fac_int_u, fac_int_u.begin())
    //   );
    //   std::set_intersection(
    //     min_v.begin(), min_v.end(),
    //     fac_set.begin(), fac_set.end(),
    //     std::inserter(fac_int_v, fac_int_v.begin())
    //   );
    //   if(fac_int_u.size() > 0) {
    //     u_complexity += 1;
    //   }
    //   if(fac_int_v.size() > 0) {
    //     v_complexity += 1;
    //   }
    // }

    if(u_complexity == v_complexity) {
      if(min_v.size() < min_u.size()) {
        return {min_v, true};
      } else {
        return {min_u, false};
      }
    } else if(u_complexity <= v_complexity) {
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

markov_chain markov::make_chain(
  MRF mrf, vertex_names source, vertex_names sink, 
  vertex_names globals,
  const map<string, Vertex>& param_vertices,
  std::function<float(std::set<std::string>)> LC, double y_cut
) {

  cout << "Removing globals... ";
  source = set_minus(source, globals);
  sink = set_minus(sink, globals);
  cout << "done." << endl;

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

    cout << "Separating:" << endl;
    write_set(cur_start);
    cout << endl;
    write_set(cur_end);
    cout << endl << endl;

    auto separator_data = minimal_separator(mrf, cur_start, cur_end, param_vertices, LC);
    separator = separator_data.first;
    closer_to_v = separator_data.second;

    float sep_complexity = LC(separator);
    // double sep_complexity = 0;
    // for (const auto& fac : lik_facs) {
    //   const std::set<std::string>& fac_set = fac.second;
    //   std::set<std::string> fac_int {};
    //   std::set_intersection(
    //     separator.begin(), separator.end(),
    //     fac_set.begin(), fac_set.end(),
    //     std::inserter(fac_int, fac_int.begin())
    //   );
    //   if(fac_int.size() > 0) {
    //     sep_complexity += 1;
    //   }
    // }
    // sep_complexity = sep_complexity / static_cast<double>(lik_facs.size());

    // set<string> y_int;
    // set_intersection(y_names.begin(), y_names.end(),
    //                  separator.begin(), separator.end(),
    //                  std::inserter(y_int, y_int.begin()));
    // double y_perc = static_cast<double>(y_int.size()) / static_cast<double>(num_ys);

    if(separator.size() == 0 || sep_complexity >= y_cut) {
      cout << "-----------------------" << endl;
      if(separator.size() == 0) {
        cout << "SEPARATION IMPOSSIBLE" << endl;
      } else {
        cout << "COMPLEXITY EXCEEDED" << endl;
      }
      cout << "-----------------------" << endl;
      separable = false;
    }

  }

  for(auto& link: chain) {
    link.insert(globals.begin(), globals.end());
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
  std::function<float(std::set<std::string>)> LC, double y_cut = 1
) {
  int num_leaves = leaves.size();
  vector<markov_chain> chains(num_leaves);
  vector<markov_chain::iterator> chain_it(num_leaves);
  for(int ci = 0; ci < num_leaves; ++ci) {
    chains[ci] = markov::make_chain(mrf, { root }, leaves[ci], globals, param_vertices, LC, y_cut);
    chain_it[ci] = chains[ci].begin();
  }

  stack<Node> node_stack;
  unique_ptr<MTree> markov_tree = make_unique<MTree>(0);

  vertex_names params = { root };
  auto name_hash = get_id(root);
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
          double ered = adj_r_squared(chain_parameters, root, stan_matrix, stan_vars);
          auto name_hash = get_id(std::reduce<vertex_names::iterator, string>(chain_parameters.begin(), chain_parameters.end(), ""));
          Node new_node = add_vertex({ 
            .name = name_hash,
            .parameters = chain_parameters,
            .ered = ered,
            .depth = cur_depth + 1,
            .chain_nums = { ci }
          }, *markov_tree);
          cout << "Connecting " << print_set((*markov_tree)[cur_node].parameters) 
               << " to " << print_set((*markov_tree)[new_node].parameters) << "." << endl;
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

Node locate_node(MTree& tree, const Node& root, int node_name) {

  std::queue<Node> node_queue {};
  node_queue.push(root);
  bool not_found = true;
  std::optional<Node> ex_node = std::nullopt;
  while(node_queue.size() > 0 && not_found) {
    Node cur_node = node_queue.front();
    node_queue.pop();

    auto out_it = out_edges(cur_node, tree);
    for_each(out_it.first, out_it.second, [&](Branch branch) {
      Node child_node = target(branch, tree);
      if(tree[child_node].name == node_name) {
        not_found = false;
        ex_node = child_node;
      } else if(not_found) {
        node_queue.push(child_node);
      }
    });
  }

  if(ex_node == nullopt) {
    cout << "Could not locate node! Cannot extrude branch." << endl;
    throw new std::out_of_range("Could not locate node! Cannot extrude branch.");
  } else {
    auto node = ex_node.value();
    return(node);
  }
}

std::pair<Node, vector<Node>> locate_node_depth_first(MTree& tree, const Node& root, int node_name) {

  int depth = 0;
  vector<int> ancestors(2);
  vector<Node> ancestor_nodes(2);
  std::stack<std::pair<Node, int>> node_queue {};
  node_queue.push(std::make_pair(root, 0));
  bool not_found = true;
  std::optional<Node> ex_node = std::nullopt;
  while(node_queue.size() > 0 && not_found) {
    auto [cur_node, cur_depth] = node_queue.top();
    node_queue.pop();
    if(ancestors.size() < cur_depth + 1) {
      ancestors.resize(cur_depth + 1);
      ancestor_nodes.resize(cur_depth + 1);
    }
    ancestors[cur_depth] = tree[cur_node].name;
    ancestor_nodes[cur_depth] = cur_node;

    auto out_it = out_edges(cur_node, tree);
    for_each(out_it.first, out_it.second, [&](Branch branch) {
      Node child_node = target(branch, tree);
      if(tree[child_node].name == node_name) {
        not_found = false;
        ex_node = child_node;
        ancestors.resize(cur_depth + 2);
        ancestor_nodes.resize(cur_depth + 2);
        ancestors[cur_depth + 1] = tree[child_node].name;
        ancestor_nodes[cur_depth + 1] = child_node;
      } else if(not_found) {
        node_queue.push(std::make_pair(child_node, cur_depth + 1));
      }
    });
  }

  if(ex_node == nullopt) {
    cout << "Could not locate node! Cannot extrude branch." << endl;
    throw new std::out_of_range("Could not locate node! Cannot extrude branch.");
  } else {
    auto node = ex_node.value();
    return(std::make_pair(node, ancestor_nodes));
  }
}

std::set<vector<Node>> find_leaf_paths(MTree& tree, const Node root) {

  set<vector<Node>> all_ancestors;

  int depth = 0;
  vector<Node> ancestor_nodes(2);
  std::stack<std::pair<Node, int>> node_queue {};
  node_queue.push(std::make_pair(root, 0));
  while(node_queue.size() > 0) {
    auto [cur_node, cur_depth] = node_queue.top();
    node_queue.pop();
    if(ancestor_nodes.size() < cur_depth + 1) {
      ancestor_nodes.resize(cur_depth + 1);
    }
    ancestor_nodes[cur_depth] = cur_node;

    auto out_it = out_edges(cur_node, tree);
    for_each(out_it.first, out_it.second, [&](Branch branch) {
      Node child_node = target(branch, tree);
      auto child_out = out_edges(child_node, tree);
      if(child_out.first == child_out.second) {
        ancestor_nodes.resize(cur_depth + 2);
        ancestor_nodes[cur_depth + 1] = child_node;
        vector<Node> leaf_path(ancestor_nodes);
        all_ancestors.insert(leaf_path);
      } else {
        node_queue.push(std::make_pair(child_node, cur_depth + 1));
      }
    });
  }

  cout << "Returning leaf ancestors of length " << to_string(all_ancestors.size()) << "." << endl;
  return(all_ancestors);

}

void markov::divide_branch(
  MTree& tree, const Node& root, 
  int node_name, vertex_names params_kept, 
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars
) {
  cout << "Beginning divide branch..." << endl;

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

  if(split_nodes == nullopt) {
    cout << "Could not locate child node! Cannot divide branch." << endl;
    throw new std::out_of_range("Could not locate child node! Cannot divide branch.");
  } else {
    cout << "Modifying tree..." << endl;
    auto [par_node, child_node] = split_nodes.value();
    remove_edge(par_node, child_node, tree);

    auto child_params = tree[child_node].parameters;
    params_kept.insert(child_params.begin(), child_params.end());
    string root_name = *tree[root].parameters.begin();
    double ered = adj_r_squared(params_kept, root_name, stan_matrix, stan_vars);
    auto name_hash = get_id(std::reduce<vertex_names::iterator, string>(params_kept.begin(), params_kept.end(), ""));
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

void markov::auto_divide(
  MTree& tree, const Node& root, 
  int node_name,
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars
) {
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

  if(split_nodes == nullopt) {
    cout << "Could not locate child node! Cannot divide branch." << endl;
    throw new std::out_of_range("Could not locate child node! Cannot divide branch.");
  } else {
    auto [par_node, child_node] = split_nodes.value();
    remove_edge(par_node, child_node, tree);

    string root_name = *tree[root].parameters.begin();
    auto child_params = tree[child_node].parameters;

    auto par_params = tree[par_node].parameters;
    map<string, set<string>> par_param_prefixes_map;
    for(auto param: par_params) {
      auto index_loc = param.find("[");
      string param_prefix = param.substr(0, index_loc);
      set<string> param_map;
      try {
        par_param_prefixes_map.at(param_prefix).insert(param);
      } catch (out_of_range _err) {
        param_map.insert(param);
        par_param_prefixes_map.insert(make_pair(param_prefix, param_map));
      }
    }
  
    set<string> best_params;
    double best_ered = 2;
    for(auto& [prefix, params]: par_param_prefixes_map) {
      params.insert(child_params.begin(), child_params.end());
      double ered = adj_r_squared(params, root_name, stan_matrix, stan_vars);
      if(ered < best_ered) {
        best_ered = ered;
        best_params = params;
      }
    }

    auto name_hash = get_id(std::reduce<vertex_names::iterator, string>(best_params.begin(), best_params.end(), ""));
    Node split_node = add_vertex({ 
      .name = name_hash,
      .parameters = best_params,
      .ered = best_ered,
      .depth = tree[par_node].depth + 1,
      .chain_nums = { }
    }, tree);

    add_edge(par_node, split_node, tree);
    add_edge(split_node, child_node, tree);

    // Fix depth?
  }
}

void markov::extrude_branch(
  MTree& tree, const Node& root, 
  int node_name, vertex_names params_kept, 
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars
) {

  auto node = locate_node(tree, root, node_name);

  string root_name = *tree[root].parameters.begin();
  double ered = adj_r_squared(params_kept, root_name, stan_matrix, stan_vars);
  auto name_hash = get_id(std::reduce<vertex_names::iterator, string>(params_kept.begin(), params_kept.end(), ""));

  Node new_node = add_vertex({ 
    .name = name_hash,
    .parameters = params_kept,
    .ered = ered,
    .depth = tree[node].depth + 1,
    .chain_nums = { }
  }, tree);

  add_edge(node, new_node, tree);

}

void markov::delete_node(
  MTree& tree, const Node& root,
  int node_name
) {
  auto [node, node_anc] = locate_node_depth_first(tree, root, node_name);
  if(node_anc.size() < 2) {
    return;
  }
  Node parent_node = node_anc[node_anc.size() - 2];

  auto [branch_it, branch_end] = out_edges(node, tree);
  while(branch_it != branch_end) {
    add_edge(parent_node, target(*branch_it, tree), tree);
    branch_it = std::next(branch_it);
  }

  clear_vertex(node, tree);
  remove_vertex(node, tree);
}

void markov::merge_nodes(
  MRF mrf, const vertex_names& globals, VertexMap& param_vertices,
  MTree& tree, const Node& root, 
  int node_name, int alt_node_name,
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  std::function<float(std::set<std::string>)> LC
) {
  auto [node, node_anc] = locate_node_depth_first(tree, root, node_name);
  auto [alt_node, alt_node_anc] = locate_node_depth_first(tree, root, alt_node_name);

  auto anb = alt_node_anc.begin();
  auto ane = alt_node_anc.end();

  Node parent_node = root;
  set<string> pre_params;

  for(int ai = node_anc.size(); ai > 0; --ai) {
    Node anc_node = node_anc[ai - 1];
    cout << "Checking if " << to_string(tree[anc_node].name) << " is a common parent." << endl;
    auto find_res = std::find_if(anb, ane, [&tree, &anc_node](Node alt_node){ 
      return(tree[alt_node].name == tree[anc_node].name); 
    });
    if(find_res != ane) {
      parent_node = anc_node;
      for(int aj = 0; aj < ai; ++aj){
        auto node_params = tree[node_anc[aj]].parameters;
        pre_params.insert(node_params.begin(), node_params.end());
      }
      break;
    }
  }
  
  vertex_names child_params_1 = tree[node].parameters;
  vertex_names child_params_2 = tree[alt_node].parameters;
  vertex_names child_params;
  child_params.insert(child_params_1.begin(), child_params_1.end());
  child_params.insert(child_params_2.begin(), child_params_2.end());

  auto new_chain = make_chain(mrf, pre_params, child_params, globals, param_vertices, LC, 1);

  string root_param = *tree[root].parameters.begin();
  Node prev_node = parent_node;
  std::for_each(std::next(new_chain.begin()), new_chain.end(), [&](vertex_names& param_names) {
    int name_hash = get_id(std::reduce<vertex_names::iterator, string>(param_names.begin(), param_names.end(), ""));
    Node new_node = add_vertex({
      .name = name_hash,
      .parameters = param_names,
      .ered = adj_r_squared(param_names, root_param, stan_matrix, stan_vars),
      .depth = tree[prev_node].depth + 1,
      .chain_nums = {}
    }, tree);
    add_edge(prev_node, new_node, tree);
    prev_node = new_node;
  });

  // int copy_hash = get_id(
  //   std::to_string(tree[node].name) + "__copy"
  // );
  // Node node_copy = add_vertex({
  //   .name = copy_hash,
  //   .parameters = child_params_1,
  //   .ered = tree[node].ered,
  //   .depth = tree[node].depth,
  //   .chain_nums = {}
  // }, tree);

  // int alt_copy_hash = get_id(
  //   std::to_string(tree[alt_node].name) + "__copy"
  // );
  // Node alt_node_copy = add_vertex({
  //   .name = alt_copy_hash,
  //   .parameters = child_params_2,
  //   .ered = tree[alt_node].ered,
  //   .depth = tree[alt_node].depth,
  //   .chain_nums = {}
  // }, tree);

  Node child1_copy = add_vertex({
    .name = get_id(std::reduce<vertex_names::iterator, string>(child_params_1.begin(), child_params_1.end(), "")),
    .parameters = child_params_1,
    .ered = tree[node].ered,
    .depth = tree[prev_node].depth + 1,
    .chain_nums = {}
  }, tree);
  Node child2_copy = add_vertex({
    .name = get_id(std::reduce<vertex_names::iterator, string>(child_params_2.begin(), child_params_2.end(), "")),
    .parameters = child_params_2,
    .ered = tree[alt_node].ered,
    .depth = tree[prev_node].depth + 1,
    .chain_nums = {}
  }, tree);
  add_edge(prev_node, child1_copy, tree);
  add_edge(prev_node, child2_copy, tree);
  // remove_edge(node_anc[node_anc.size() - 2], node, tree);
  // remove_edge(alt_node_anc[alt_node_anc.size() - 2], alt_node, tree);
}

void markov::auto_merge(
  MRF mrf, const vertex_names& globals, VertexMap& param_vertices,
  MTree& tree, const Node& root, 
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  int merge_depth, std::function<float(std::set<std::string>)> LC
) {
  auto leaf_anc = find_leaf_paths(tree, root);
  std::map<int, vector<Node>> node_groups;
  for(const vector<Node> path: leaf_anc) {
    int max_anc = static_cast<int>(path.size()) - 1;
    int merge_point = max(max_anc - merge_depth, 0);
    int merge_name = tree[path[merge_point]].name;
    if(node_groups.find(merge_name) == node_groups.end()) {
      vector<Node> map_vec;
      map_vec.reserve(leaf_anc.size());
      node_groups.insert(make_pair(merge_name, map_vec));
    }
    node_groups.at(merge_name).push_back(path[max_anc]);
  }

  double best_ered = 2;
  int best_node = 0;
  int best_alt_node = 0;
  string root_param = *tree[root].parameters.begin();

  for(const auto& [merge_key, node_group]: node_groups) {
    for(size_t ni = 0; ni < (node_group.size() - 1); ++ni) {
      for(size_t nj = ni+1; nj < node_group.size(); ++nj) {
        set<string> merge_params;
        set<string> params_1 = tree[node_group[ni]].parameters;
        set<string> params_2 = tree[node_group[nj]].parameters;
        merge_params.insert(params_1.begin(), params_1.end());
        merge_params.insert(params_2.begin(), params_2.end());
        double merge_ered = adj_r_squared(merge_params, root_param, stan_matrix, stan_vars);
        if(merge_ered < best_ered) {
          best_node = tree[node_group[ni]].name;
          best_alt_node = tree[node_group[nj]].name;
          best_ered = merge_ered;
        }
      }
    }
  }

  cout << "Merging best pair..." << endl;
  merge_nodes(mrf, globals, param_vertices, tree, root, best_node, best_alt_node, stan_matrix, stan_vars, LC);
}

void markov::auto_merge2(
  MRF mrf, const vertex_names& globals, VertexMap& param_vertices,
  MTree& tree, const Node& root, 
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  int merge_depth, std::function<float(std::set<std::string>)> LC
) {

  string root_param = *tree[root].parameters.begin();

  int best_node = 0;
  int best_alt_node = 0;
  double best_rel_ered = 100000;
  
  stack<Node> node_stack;
  node_stack.push(root);
  while(node_stack.size() > 0) {
    auto cur_node = node_stack.top();
    node_stack.pop();

    auto p_params = tree[cur_node].parameters;

    auto branches = out_edges(cur_node, tree);
    for(auto branch1 = branches.first; branch1 != branches.second; branch1 = std::next(branch1)) {
      Node child1 = target(*branch1, tree);
      node_stack.push(child1);
      for(auto branch2 = std::next(branch1); branch2 != branches.second; branch2 = std::next(branch2)) {
        Node child2 = target(*branch2, tree);
        auto c1_params = tree[child1].parameters;
        auto c2_params = tree[child2].parameters;
        
        set<string> c_params;
        c_params.insert(c1_params.begin(), c1_params.end());
        c_params.insert(c2_params.begin(), c2_params.end());

        if(!std::includes(c_params.begin(), c_params.end(), p_params.begin(), p_params.end())) {
          double c1_ered = tree[child1].ered.value();
          double c2_ered = tree[child2].ered.value();
          double merge_ered = adj_r_squared(c_params, root_param, stan_matrix, stan_vars);
          double rel_ered = merge_ered / min(c1_ered, c2_ered);
          if(rel_ered < best_rel_ered) {
            best_node = tree[child1].name;
            best_alt_node = tree[child2].name;
            best_rel_ered = rel_ered;
          }
        }
      }
    }
  }

  if(best_node != best_alt_node) {
    merge_nodes(mrf, globals, param_vertices, tree, root, best_node, best_alt_node, stan_matrix, stan_vars, LC);
  } else {
    cout << "No eligible mergers!" << endl;
  }
}

