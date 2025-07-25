#include <parameter_graph.hpp>
#include <read_stan.hpp>
#include <Eigen/Dense>

namespace markov {

  typedef std::list<vertex_names> markov_chain;

  // vertex_names minimal_separator_u(MRF mrf, vertex_names u, vertex_names v, const VertexMap& param_vertices);

  // pair<vertex_names, bool> minimal_separator(MRF mrf, vertex_names u, vertex_names v, const VertexMap& param_vertices);

  markov_chain make_chain(MRF mrf, vertex_names source, vertex_names sink, const std::map<std::string, Vertex>& param_vertices, double y_cut);

  std::pair<std::unique_ptr<MTree>, Node> make_tree(
    MRF mrf, const std::string& root, const std::vector<vertex_names> leaves,
    const vertex_names& globals, VertexMap& param_vertices,
    const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
    double y_cut);


  void divide_branch(MTree& tree, const Node& root, size_t node_name, vertex_names params_kept, const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars);
}