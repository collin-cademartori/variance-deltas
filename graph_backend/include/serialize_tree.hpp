#include <optional>
#include <set>
#include <string>
#include <parameter_graph.hpp>

std::string serialize_tree(const Node& root, const MTree& tree, const std::set<std::string>& globals, double global_limit, std::optional<std::string> sid);
