#include <iostream>
#include <string>

#include <boost/process/v2.hpp>
#include <boost/asio.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

#include <run_model_parser.hpp>

namespace proc = boost::process::v2;
namespace asio = boost::asio;
using namespace std;

std::optional<ParserOutput> run_model_parser(
  const std::string& model_file,
  const std::string& data_file
) {
  // Get executable directory for finding sibling executables
  boost::filesystem::path exec_path = boost::dll::program_location();
  boost::filesystem::path exec_dir = exec_path.parent_path();
#ifdef _WIN32
  boost::filesystem::path model_parser_path = exec_dir / "model_parser.exe";
#else
  boost::filesystem::path model_parser_path = exec_dir / "model_parser";
#endif

  // Check that model_parser exists before trying to run it
  if (!boost::filesystem::exists(model_parser_path)) {
    cerr << "Error: model_parser executable not found at: " << model_parser_path << endl;
    return std::nullopt;
  }

  asio::io_context ioc;
  asio::readable_pipe interp_pipe{ioc};

  cout << "Running parser: " << model_parser_path << endl;

  proc::process interp_proc(
    ioc,
    model_parser_path.string(),
    { model_file, "-d", data_file },
    proc::process_stdio({{}, interp_pipe, {}})
  );

  string interp_data;
  boost::system::error_code pipe_code;
  asio::read(interp_pipe, asio::dynamic_buffer(interp_data), pipe_code);

  if (pipe_code != asio::error::eof) {
    cerr << "Error reading model_parser output: " << pipe_code.message() << endl;
    return std::nullopt;
  }

  int exit_code = interp_proc.wait();

  if (exit_code != 0) {
    cerr << "Error: model_parser exited with code " << exit_code << endl;
    if (!interp_data.empty()) {
      cerr << "Parser output:\n" << interp_data << endl;
    }
    return std::nullopt;
  }

  cout << "Parser ran successfully" << endl;

  // Check for empty output
  if (interp_data.empty()) {
    cerr << "Error: model_parser produced no output." << endl;
    return std::nullopt;
  }

  // Split output into factor graph data and tree data
  const size_t tree_begin = interp_data.find("\n--");
  if (tree_begin == string::npos) {
    cerr << "Error: Parser output missing '--' delimiter. Malformed output." << endl;
    cerr << "Output was:\n" << interp_data.substr(0, 500);
    if (interp_data.size() > 500) cerr << "... (truncated)";
    cerr << endl;
    return std::nullopt;
  }

  ParserOutput output;
  output.fg_data = interp_data.substr(0, tree_begin);
  output.tree_data = interp_data.substr(tree_begin + 4);

  return output;
}
