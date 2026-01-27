#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>

#include <parse_options.hpp>

namespace options = boost::program_options;
using namespace std;

ParseResult parse_options(int argc, char* argv[]) {
  options::options_description ops_desc("Command line options.");
  auto ops = ops_desc.add_options();
  ops("help", "print this help message")
  ("model_file,M", options::value<string>()->required(), "specify the model file")
  ("data_file,D", options::value<string>()->required(), "specify the data file")
  ("stan_file_prefix,S", options::value<string>()->required(), "specify the prefix of Stan's MCMC output CSV files")
  ("num_chains,N", options::value<int>()->required(), "specify the number of MCMC chains, i.e. the number of MCMC CSV files to read")
  ("port,P", options::value<int>()->default_value(8765), "specify the WebSocket server port (default: 8765)");

  options::variables_map user_input;

  try {
    options::store(options::parse_command_line(argc, argv, ops_desc), user_input);

    if (user_input.count("help")) {
      std::cout << ops_desc << endl;
      return {std::nullopt, 0};
    }

    options::notify(user_input);
  } catch (options::required_option& err) {
    std::cerr << "Must specify options " << err.what() << endl << "Run with --help for details." << endl;
    return {std::nullopt, 1};
  }

  Config config;
  config.model_file = user_input["model_file"].as<string>();
  config.data_file = user_input["data_file"].as<string>();
  config.stan_file_prefix = user_input["stan_file_prefix"].as<string>();
  config.num_chains = user_input["num_chains"].as<int>();
  config.ws_port = user_input["port"].as<int>();

  // Verify input files exist
  bool files_missing = false;

  if (!std::filesystem::exists(config.model_file)) {
    std::cerr << "Error: Model file does not exist: " << config.model_file << endl;
    files_missing = true;
  }

  if (!std::filesystem::exists(config.data_file)) {
    std::cerr << "Error: Data file does not exist: " << config.data_file << endl;
    files_missing = true;
  }

  // Check if Stan output files exist (check for first chain)
  string first_chain_file = config.stan_file_prefix + "1.csv";
  if (!std::filesystem::exists(first_chain_file)) {
    std::cerr << "Error: Stan output file does not exist: " << first_chain_file << endl;
    std::cerr << "       (Looking for files with prefix: " << config.stan_file_prefix << ")" << endl;
    files_missing = true;
  } else {
    // Check if all chains exist
    for (int i = 1; i <= config.num_chains; ++i) {
      string chain_file = config.stan_file_prefix + to_string(i) + ".csv";
      if (!std::filesystem::exists(chain_file)) {
        std::cerr << "Error: Stan output file does not exist: " << chain_file << endl;
        files_missing = true;
      }
    }
  }

  if (files_missing) {
    std::cerr << endl << "Please check the file paths and try again." << endl;
    return {std::nullopt, 1};
  }

  std::cout << endl << "All file paths resolved, proceeding." << endl;
  return {config, 0};
}
