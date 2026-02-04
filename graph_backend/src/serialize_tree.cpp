#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <parameter_graph.hpp>

using namespace std;
using namespace boost;

pair<string,string> serialize_node(const Node& node, const MTree& tree, string parent_name) {
  vector<string> ordered_params {};
  for(const string param: tree[node].parameters) {
    ordered_params.push_back(param);
  }
  sort(ordered_params.begin(), ordered_params.end());

  //string set_name = "{";
  string set_array = "[";
  for(size_t pi = 0; pi < ordered_params.size(); ++pi) {
    string param = ordered_params[pi];
    //set_name += param;
    set_array += ("\"" + param + "\"");
    if(pi + 1 < ordered_params.size()) {
      //set_name += ", ";
      set_array += ",";
    }
  }
  //set_name += "}";
  set_array += "]";

  string node_name = to_string(tree[node].name);

  string serialized_str = "{";
  serialized_str += "\"name\":\"" + node_name + "\",";
  serialized_str += "\"params\":" + set_array + ",";
  serialized_str += "\"ered\":" + to_string(tree[node].ered.value()) + ",";
  serialized_str += "\"parent\":\"" + parent_name + "\"";
  serialized_str += "}";

  return make_pair(serialized_str, node_name);
}

string serialize_set(const set<string>& str_set) {
  string globals_string = "[";
  for(auto git = str_set.begin(); git != str_set.end(); git = std::next(git)) {
    globals_string += "\"" + *git + "\"";
    if(std::next(git) != str_set.end()) {
      globals_string += ",";
    }
  }
  globals_string += "]";

  return(globals_string);
}

string serialize_tree(
  const Node& root, const MTree& tree,
  const set<string>& globals, double global_limit,
  std::optional<std::string> sid
) {

  queue<pair<Node, string>> node_queue{};

  string parent_name = "";
  auto [root_str, root_name] = serialize_node(root, tree, parent_name);
  string serialized_tree = "[" + root_str + ",";

  node_queue.push(make_pair(root, root_name));

  while(node_queue.size() > 0) {
    auto [cur_node, cur_name] = node_queue.front();
    node_queue.pop();

    auto out_it = out_edges(cur_node, tree);
    for_each(out_it.first, out_it.second, [&](Branch branch) {
      const Node& child = target(branch, tree);
      auto [child_str, child_name] = serialize_node(child, tree, cur_name);
      serialized_tree += child_str + ",";
      node_queue.push(make_pair(child, child_name));
    });
  }

  serialized_tree = serialized_tree.substr(0, serialized_tree.size() - 1);
  serialized_tree += "]";

  string serialized_globals = serialize_set(globals);

  string tree_str = "{\"type\":\"tree\",";
  tree_str += "\"tree\":" + serialized_tree + ",";
  tree_str += "\"globals\":" + serialized_globals + ",";
  tree_str += "\"global_limit\":" + std::to_string(global_limit) + ",";
  tree_str += "\"groups\":{}";  // Groups are now managed by frontend
  if(sid) {
    tree_str += ",\"sid\":\"" + *sid + "\"";
  }
  tree_str += "}";
  cout << tree_str << endl;
  return tree_str;
}
