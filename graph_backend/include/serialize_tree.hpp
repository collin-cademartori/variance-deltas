#include <parameter_graph.hpp>

std::string serialize_tree(const Node& root, const MTree& tree, const std::set<std::string>& globals, double global_limit, const std::map<std::string, std::set<std::string>>& groups);

std::string serialize_groups(const std::map<std::string, std::set<std::string>> groups, bool wrap_message);