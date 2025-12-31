#include<sstream>
#include<iostream>

#include<read_tree_data.hpp>

std::pair<std::string, leaves_t> read_tree_data(std::string tree_data) {

  std::string root_name;
  leaves_t leaves;

  std::istringstream tree_stream(tree_data);
  getline(tree_stream, root_name);

  std::string leaf_line;
  while(getline(tree_stream, leaf_line)) {
    // std::cout << "Leaf set: " << leaf_line << std::endl;
    std::set<std::string> leaf_set{};
    int leaf_pos = 0;
    int leaf_pos_next = 0;
    do {
      leaf_pos_next = leaf_line.find(", ", leaf_pos);
      std::string leaf_name = leaf_line.substr(leaf_pos, leaf_pos_next - leaf_pos);
      // std::cout << "Leaf: " << leaf_name << std::endl;
      leaf_set.emplace(leaf_name);
      leaf_pos = leaf_pos_next + 2;
    } while (leaf_pos_next != std::string::npos);
    leaves.emplace_back(leaf_set);
  }

  return std::make_pair(root_name, leaves);
}