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

  string set_name = "{";
  string set_array = "[";
  for(size_t pi = 0; pi < ordered_params.size(); ++pi) {
    string param = ordered_params[pi];
    set_name += param;
    set_array += ("\"" + param + "\"");
    if(pi + 1 < ordered_params.size()) {
      set_name += ", ";
      set_array += ",";
    }
  }
  set_name += "}";
  set_array += "]";

  string serialized_str = "{";
  serialized_str += "\"name\":\"" + set_name + "\",";
  serialized_str += "\"params\":" + set_array + ",";
  serialized_str += "\"ered\":" + to_string(tree[node].ered.value()) + ",";
  serialized_str += "\"parent\":\"" + parent_name + "\"";
  serialized_str += "}";
  
  return make_pair(serialized_str, set_name);
}

string serialize_tree(const Node& root, const MTree& tree) {

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

  cout << "Returning serialization." << endl;
  return serialized_tree;
}