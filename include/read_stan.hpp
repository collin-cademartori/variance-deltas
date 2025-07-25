#pragma once

#include <map>
#include <Eigen/Dense>

struct standata { 
  std::unique_ptr<Eigen::MatrixXd> samples;
  std::map<std::string, int> vars;
};

standata read_stan_file(std::string file_name, int num_chains);