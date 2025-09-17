#include <iostream>
#include <fstream>
#include <iterator>

#include <read_lik.hpp>

lik_facs read_lik(std::string lik_file_path) {
  std::cout << "Reading MRF graph." << std::endl; 

  lik_facs factors {};

  // Read LIK file
  std::ifstream lik_file(lik_file_path);
  if (!lik_file.is_open()) {
      std::cerr << "Could not open lik file, aborting." << std::endl;
  }

  bool reading_factor_name = false;
  std::string lik_line;
  while(getline(lik_file, lik_line)) {
    lik_facs::iterator inserted_fac = factors.insert({lik_line, {}}).first;
    std::set<std::string>& fac_params = inserted_fac->second;
    while(getline(lik_file, lik_line) && lik_line != "---") {
      fac_params.insert(lik_line);
    }
  }

  lik_file.close();

  return factors;
}