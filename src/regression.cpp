#include <Eigen/Dense>
#include <read_stan.hpp>
#include <set>

using namespace std;
using Eigen::MatrixXd;
using Eigen::ArrayXd;
using Eigen::VectorXd;
using Eigen::all;
using Eigen::seqN;

MatrixXd predictor_matrix(const MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars, set<string> pred_names, int poly, bool interactions){
  vector<int> var_indices(pred_names.size());
  int pi = 0;
  for(const string& pred_name: pred_names) {
    var_indices[pi] = stan_vars.at(pred_name);
    pi++;
  }

  VectorXd intercept = (ArrayXd::Zero(stan_matrix.rows()) + 1).matrix();

  MatrixXd pred_matrix = stan_matrix(all, var_indices);
  MatrixXd out_matrix(stan_matrix.rows(), var_indices.size() + 1);
  out_matrix << intercept, pred_matrix;

  int C = pred_matrix.cols();
  int R = pred_matrix.rows();
  if(poly > 1) {
    MatrixXd poly_matrix(R, (poly * C) + 1);
    poly_matrix(all, seqN(0, C + 1)) = out_matrix;
    for(int pi = 2; pi <= poly; ++pi) {
      poly_matrix(all, seqN((pi - 2) * C + 2, C)) = pred_matrix.array().pow(pi).matrix();
    }
    out_matrix = poly_matrix;
  }

  if(interactions) {
    int num_int = C * (C - 1) / 2;
    int cur_col = 0;
    int OC = out_matrix.cols();
    MatrixXd int_matrix(R, OC + num_int);
    int_matrix(all, seqN(0, OC)) = out_matrix;
    for(int ii = 1; ii < C; ++ii) {
      for(int jj = 0; jj < ii; ++jj) {
        int_matrix(all, OC + cur_col) = pred_matrix(all, ii).cwiseProduct(pred_matrix(all, jj));
        ++cur_col;
      }
    }
    out_matrix = int_matrix;
  }
  return out_matrix;
}

double adj_r_squared(set<string> predictor_names, std::string response_name, const MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars) {

  int num_observations = stan_matrix.rows();
  VectorXd response = stan_matrix(all, stan_vars.at(response_name));
  MatrixXd predictors = predictor_matrix(stan_matrix, stan_vars, predictor_names, 1, true);
  int num_predictors = predictors.cols() - 1;

  VectorXd coefs = predictors.colPivHouseholderQr().solve(response);
  VectorXd predictions = predictors * coefs;

  double SSR = (response - predictions).squaredNorm();
  double SST = (response.array() - response.mean()).matrix().squaredNorm();
  double adj_rsq = (SSR / SST) * ((static_cast<double>(num_observations) - 1) / (static_cast<double>(num_observations) - static_cast<double>(num_predictors) - 1));

  return adj_rsq;
} 