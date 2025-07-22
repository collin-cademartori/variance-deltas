#include <Eigen/Dense>
#include <read_stan.hpp>

using namespace std;
using Eigen::MatrixXd;
using Eigen::ArrayXd;
using Eigen::VectorXd;
using Eigen::all;
using Eigen::seqN;

MatrixXd predictor_matrix(const standata& data, vector<string> pred_names, int poly, bool interactions){
  vector<int> var_indices(pred_names.size());
  for(int pi = 0; pi < pred_names.size(); ++pi) {
    var_indices[pi] = data.vars.at(pred_names[pi]);
  }

  VectorXd intercept = (ArrayXd::Zero(data.samples.rows()) + 1).matrix();

  MatrixXd pred_matrix = data.samples(all, var_indices);
  MatrixXd out_matrix(data.samples.rows(), var_indices.size() + 1);
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

double adj_r_squared(vector<string> predictor_names, string response_name, standata data) {

  int num_observations = data.samples.rows();
  VectorXd response = data.samples(all, data.vars[response_name]);
  MatrixXd predictors = predictor_matrix(data, predictor_names, 1, true);
  int num_predictors = predictors.cols() - 1;

  VectorXd coefs = predictors.colPivHouseholderQr().solve(response);
  VectorXd predictions = predictors * coefs;

  double SSR = (response - predictions).squaredNorm();
  double SST = (response.array() - response.mean()).matrix().squaredNorm();
  double adj_rsq = (SSR / SST) * ((static_cast<double>(num_observations) - 1) / (static_cast<double>(num_observations) - static_cast<double>(num_predictors) - 1));

  return adj_rsq;
} 