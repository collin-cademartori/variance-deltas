#include <iostream>
#include <fstream>
#include <map>
#include <Eigen/Dense>

using namespace std;
using Eigen::MatrixXd;
using Eigen::ArrayXd;
using Eigen::VectorXd;
using Eigen::Map;
using Eigen::all;
using Eigen::seqN;

struct standata { 
  MatrixXd samples;
  map<string, int> vars;
};

vector<int> dot_pos(string name) {
  vector<int> pos;
  for(int si = 0; si < name.size(); ++si) {
    if(name[si] == '.') pos.push_back(si);
  }
  return(pos);
}
 
standata read_stan_file(string file_name, int num_chains) {
  vector<double> stan_data;
  vector<string> stan_names;
  int sample_size = 0;

  for(int i = 1; i < num_chains + 1; ++i) {

    ifstream stan_file("../data/" + file_name + to_string(i) + ".csv");

    if (!stan_file.is_open()) {
      throw runtime_error("Could not open stan csv file, aborting.");
    }

    bool header_done = false;
    string stan_file_line;

    while(getline(stan_file, stan_file_line)) {
      bool is_data = stan_file_line[0] != '#';
      if(is_data && header_done) {
        stringstream stan_line_data(stan_file_line);
        string stan_file_cell;
        while (getline(stan_line_data, stan_file_cell, ',')) {
          stan_data.push_back(stod(stan_file_cell));
        }
        ++sample_size;
      } else if (is_data && !header_done) {
        if(stan_names.size() == 0) {
          stringstream stan_line_data(stan_file_line);
          string stan_file_cell;
          while (getline(stan_line_data, stan_file_cell, ',')) {
            stan_names.push_back(stan_file_cell);
          }
        }
        header_done = true;
      }
    }

    stan_file.close();
  }

  int num_vars = stan_names.size();

  Map<MatrixXd> stan_matrix_t(stan_data.data(), num_vars, sample_size);
  cout << "Finished reading in matrix..." << endl;
  MatrixXd stan_matrix = stan_matrix_t.transpose();
  cout << "Finished transposing matrix..." << endl;

  cout << "Read " << stan_matrix.rows() << " x " 
       << stan_matrix.cols() << " matrix." << endl;

  map<string, int> col_names;
  for(int ci = 0; ci < stan_names.size(); ++ci) {
    string par_name = stan_names[ci];
    vector<int> par_dots = dot_pos(par_name);
    if(par_dots.size() > 0) {
      par_name[par_dots[0]] = '[';
      for(int pi = 1; pi < par_dots.size(); ++pi) {
        par_name[par_dots[pi]] = ',';
      }
      par_name = par_name + "]";
    }
    col_names.insert(make_pair(par_name, ci));
  }

  return {
    .samples = stan_matrix,
    .vars = col_names
  };
}

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

int main(int, char* []) {
  standata data = read_stan_file("standata", 6);

  int num_observations = data.samples.rows();
  VectorXd response = data.samples(all, data.vars["mu_b[49,39]"]);

  // vector<string> predictor_names = {
  //   "mu_b[49,37]", "mu_c[90]", "mu_m[1]", "mu_pop[1]", "polling_bias[49]"
  // };
  vector<string> predictor_names = {
    "mu_pop[1]", "mu_pop[2]", "mu_pop[3]", "e_bias[37]", "mu_c[90]", "mu_m[1]", "mu_m[2]", "mu_m[3]", "polling_bias[49]", "mu_b[49,37]"
  };
  // vector<string> predictor_names;
  // for(int ndi = 1; ndi <= 54; ++ndi) {
  //   predictor_names.push_back("n_democrat_potential[" + to_string(ndi) + "]");
  // }
  MatrixXd predictors = predictor_matrix(data, predictor_names, 1, true);
  // for(int pi = 0; pi < 3; ++pi) {
  //   for(auto pred: predictors(pi, all)) {
  //     cout << to_string(pred) << ";";
  //   }
  //   cout << endl << endl;
  // }
  int num_predictors = predictors.cols() - 1;

  cout << "Predictor matrix of size " << predictors.rows() << " x " << predictors.cols() << endl;

  VectorXd coefs = predictors.colPivHouseholderQr().solve(response);
  //VectorXd coefs = predictors.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(response);
  // cout << "Coefficients are:" << endl;
  // for(auto coef: coefs) {
  //   cout << to_string(coef) << "; ";
  // }
  // cout << endl << endl;
  VectorXd predictions = predictors * coefs;

  cout << "Response:" << endl;
  for(int ri = 0; ri < 5; ++ri) {
    cout << to_string(response[ri]) << ";";
  }
  cout << endl;
  cout << "Predictions:" << endl;
  for(int ri = 0; ri < 5; ++ri) {
    cout << to_string(predictions[ri]) << ";";
  }
  cout << endl << endl;

  double SSR = (response - predictions).squaredNorm();
  double SST = (response.array() - response.mean()).matrix().squaredNorm();
  double adj_rsq = (SSR / SST) * ((static_cast<double>(num_observations) - 1) / (static_cast<double>(num_observations) - static_cast<double>(num_predictors) - 1));

  cout << "SST is " << to_string(SST) << "." << endl;
  cout << "SSR is " << to_string(SSR) << "." << endl;

  cout << "Fraction of variance unexplained is " << adj_rsq << "." << endl;
}
