#pragma once

#include <string>
#include <optional>

struct Config {
  std::string model_file;
  std::string data_file;
  std::string stan_file_prefix;
  int num_chains;
  int ws_port;
};

struct ParseResult {
  std::optional<Config> config;  // Has value if parsing succeeded
  int exit_code;                 // 0 for --help, 1 for error (only meaningful if config is nullopt)
};

// Parses command line options and validates that input files exist.
// Returns ParseResult with:
//   - config has value: proceed with program
//   - config is nullopt, exit_code 0: --help was requested
//   - config is nullopt, exit_code 1: error occurred
ParseResult parse_options(int argc, char* argv[]);
