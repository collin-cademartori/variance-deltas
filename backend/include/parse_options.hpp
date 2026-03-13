#pragma once

#include <string>
#include <optional>

struct Config {
  std::optional<std::string> model_file;   // Required unless archive_file is set
  std::optional<std::string> data_file;    // Required unless archive_file is set
  std::string stan_file_prefix;            // Always required
  int num_chains;                          // Always required
  int ws_port;
  std::optional<std::string> archive_file; // If set, load state from archive
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
