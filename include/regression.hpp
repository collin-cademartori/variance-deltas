#include <read_stan.hpp>
#include <Eigen/Dense>

double adj_r_squared(std::set<std::string> predictor_names, std::string response_name, const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars);
