#pragma once

#include <string>
#include <optional>

struct ParserOutput {
  std::string fg_data;
  std::string tree_data;
};

// Runs the model_parser subprocess and captures its output.
// Returns nullopt if the parser fails or produces malformed output.
std::optional<ParserOutput> run_model_parser(
  const std::string& model_file,
  const std::string& data_file
);
