#include<map>
#include<fstream>
#include<iostream>
#include<Eigen/Dense>

#include<read_stan.hpp>

using namespace std;
using Eigen::MatrixXd;
using Eigen::Map;

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

    ifstream stan_file(file_name + to_string(i) + ".csv");

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

  auto stan_matrix_t =  std::make_unique<Map<MatrixXd>>(stan_data.data(), num_vars, sample_size);
  cout << "Finished reading in matrix..." << endl;
  auto stan_matrix = std::make_unique<MatrixXd>(stan_matrix_t -> transpose());
  cout << "Finished transposing matrix..." << endl;

  cout << "Read " << stan_matrix -> rows() << " x " 
       << stan_matrix -> cols() << " matrix." << endl;

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
    .samples = std::move(stan_matrix),
    .vars = col_names
  };
}