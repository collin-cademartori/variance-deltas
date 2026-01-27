#include <iostream>
#include <string>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/dll/runtime_symbol_info.hpp>

#include <run_model_parser.hpp>

namespace proc = boost::process;
namespace asio = boost::asio;
using namespace std;

std::optional<ParserOutput> run_model_parser(
  const std::string& model_file,
  const std::string& data_file
) {
  // Get executable directory for finding sibling executables
  boost::filesystem::path exec_path = boost::dll::program_location();
  boost::filesystem::path exec_dir = exec_path.parent_path();
  boost::filesystem::path model_parser_path = exec_dir / "model_parser";

  asio::io_context ioc;
  asio::readable_pipe interp_pipe{ioc};

  cout << "About to run parser: " << model_parser_path << endl;

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
    cerr << "Error reading interpreter output." << endl;
    return std::nullopt;
  }

  interp_proc.wait();

  cout << "Parser ran" << endl;

  // Split output into factor graph data and tree data
  const size_t tree_begin = interp_data.find("\n--");
  if (tree_begin == string::npos) {
    cerr << "Error: Parser output missing delimiter. Malformed output." << endl;
    return std::nullopt;
  }

  ParserOutput output;
  output.fg_data = interp_data.substr(0, tree_begin);
  output.tree_data = interp_data.substr(tree_begin + 4);

  return output;
}
