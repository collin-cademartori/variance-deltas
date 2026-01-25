#include <Eigen/Dense>
#include <set>
#include <map>
#include <string>

double rf_oob_mse(
  std::set<std::string> predictor_names, std::string response_name,
  const Eigen::MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  bool sqrt_scale = true, bool split_data = true);
