#include <regression_rf.hpp>

#include <fstream>
#include <filesystem>
#include <sstream>
#include <regex>
#include <iostream>

#include <boost/process.hpp>
#include <boost/asio.hpp>

namespace proc = boost::process;
namespace asio = boost::asio;
using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::all;

// Replace characters that ranger doesn't allow in column names
static string sanitize_column_name(const string& name) {
  string result = name;
  for(char& c : result) {
    if(c == ',' || c == ';' || c == '[' || c == ']') {
      c = '_';
    }
  }
  return result;
}

// Helper to run ranger and capture output
static string run_ranger(const vector<string>& args) {
  asio::io_context ioc;
  asio::readable_pipe ranger_pipe{ioc};

  proc::process ranger_proc(
    ioc,
    "/Users/collin/.local/bin/ranger",
    args,
    proc::process_stdio({{}, ranger_pipe, {}})
  );

  string ranger_output;
  boost::system::error_code pipe_code;
  asio::read(ranger_pipe, asio::dynamic_buffer(ranger_output), pipe_code);

  if(pipe_code != asio::error::eof) {
    throw runtime_error("Error reading ranger output");
  }

  ranger_proc.wait();
  return ranger_output;
}

double rf_oob_mse(
  set<string> predictor_names, std::string response_name,
  const MatrixXd& stan_matrix, const std::map<std::string, int>& stan_vars,
  bool sqrt_scale, bool split_data
) {
  cout << "Computing RF holdout prediction error." << endl;

  // No predictors means no variance explained, so SSR/SST = 1
  if(predictor_names.empty()) {
    return 1.0;
  }

  auto temp_dir = std::filesystem::temp_directory_path();
  auto train_file = temp_dir / "ranger_train.csv";
  auto test_file = temp_dir / "ranger_test.csv";
  auto forest_file = temp_dir / "ranger_model.forest";
  auto pred_prefix = temp_dir / "ranger_pred";
  string train_file_str = train_file.string();
  string test_file_str = test_file.string();
  string forest_file_str = forest_file.string();
  string pred_prefix_str = pred_prefix.string();

  // Build column indices and sanitized names
  vector<int> pred_indices;
  vector<string> sanitized_names;

  for(const string& pred_name : predictor_names) {
    pred_indices.push_back(stan_vars.at(pred_name));
    sanitized_names.push_back(sanitize_column_name(pred_name));
  }

  int response_idx = stan_vars.at(response_name);
  string sanitized_response = sanitize_column_name(response_name);

  // Split data: first half for training, second half for test
  int num_rows = stan_matrix.rows();
  int num_train = num_rows / 2;
  int num_test = num_rows - num_train;

  // Extract test response for SST calculation
  VectorXd response_test = stan_matrix.col(response_idx).tail(num_test);
  double response_mean = response_test.mean();
  double SST = (response_test.array() - response_mean).matrix().squaredNorm();

  // Write header helper
  auto write_header = [&](ofstream& out) {
    for(size_t i = 0; i < sanitized_names.size(); ++i) {
      out << sanitized_names[i] << ",";
    }
    out << sanitized_response << "\n";
  };

  // Write training data (first half)
  {
    ofstream csv_out(train_file_str);
    write_header(csv_out);
    for(int row = 0; row < num_train; ++row) {
      for(int pred_idx : pred_indices) {
        csv_out << stan_matrix(row, pred_idx) << ",";
      }
      csv_out << stan_matrix(row, response_idx) << "\n";
    }
  }

  // Write test data (second half)
  {
    ofstream csv_out(test_file_str);
    write_header(csv_out);
    for(int row = num_train; row < num_rows; ++row) {
      for(int pred_idx : pred_indices) {
        csv_out << stan_matrix(row, pred_idx) << ",";
      }
      csv_out << stan_matrix(row, response_idx) << "\n";
    }
  }

  // Step 1: Train ranger and save forest
  run_ranger({
    "--file", train_file_str,
    "--treetype", "3",  // Regression
    "--depvarname", sanitized_response,
    "--write",
    "--outprefix", forest_file_str,
    "--verbose"
  });

  // Step 2: Predict on test data
  run_ranger({
    "--file", test_file_str,
    "--treetype", "3",  // Regression
    "--depvarname", sanitized_response,
    "--predict", forest_file_str + ".forest",
    "--outprefix", pred_prefix_str,
    "--verbose"
  });

  // Step 3: Read predictions from output file
  string pred_file_str = pred_prefix_str + ".prediction";
  VectorXd predictions(num_test);
  {
    ifstream pred_in(pred_file_str);
    if(!pred_in.good()) {
      throw runtime_error("Could not open prediction file: " + pred_file_str);
    }

    string line;
    // Skip header line if present
    getline(pred_in, line);
    if(line.find("Predictions") != string::npos || line.find("pred") != string::npos) {
      // It was a header, continue reading
    } else {
      // No header, first line is data
      predictions(0) = stod(line);
    }

    int idx = (line.find("Predictions") != string::npos || line.find("pred") != string::npos) ? 0 : 1;
    while(getline(pred_in, line) && idx < num_test) {
      predictions(idx) = stod(line);
      idx++;
    }

    if(idx != num_test) {
      throw runtime_error("Prediction count mismatch: expected " + to_string(num_test) + ", got " + to_string(idx));
    }
  }

  // Step 4: Compute SSR ourselves
  double SSR = (response_test - predictions).squaredNorm();
  double normalized = SSR / SST;

  cout << "num_test: " << num_test << ", SSR: " << SSR << ", SST: " << SST << endl;
  cout << "Normalized (SSR/SST): " << normalized << endl;

  // Clean up temp files
  std::filesystem::remove(train_file);
  std::filesystem::remove(test_file);
  std::filesystem::remove(forest_file_str + ".forest");
  std::filesystem::remove(pred_file_str);

  if(sqrt_scale) {
    return sqrt(normalized);
  } else {
    return normalized;
  }
}
