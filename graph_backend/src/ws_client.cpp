#include <simple-websocket-server/client_ws.hpp>
#include <ws_client.hpp>
#include <nlohmann/json.hpp>
#include <memory>

using namespace std;
using json = nlohmann::json;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

unique_ptr<WsClient> ws_client;

void initialize_ws_client(const string& host, int port) {
  string server_address = host + ":" + to_string(port);
  ws_client = make_unique<WsClient>(server_address);
  cout << "WebSocket client configured to connect to: " << server_address << endl;
}

enum mtype { method };

map<string, mtype> msg_types = { {"method", method} };
map<string, function<void(json, std::shared_ptr<WsClient::Connection>)>> method_handlers;

void handle_method(std::string method_name, std::function<std::optional<std::string>(json)> handler) {

  const auto handler_wrapper = [handler](json json_data, std::shared_ptr<WsClient::Connection> conn) {
    cout << "Calling inner handler!" << endl;
    auto message = handler(json_data);
    if(message != nullopt) {
      conn -> send(message.value());
    }
  };

  method_handlers.insert(make_pair(method_name, handler_wrapper));
}

void send_tree(string tree_string, WsClient::Connection& conn) {
  string msg_string = "{\"type\":\"tree\",";
  msg_string += ("\"tree\":" + tree_string + "}");
  conn.send(msg_string);
}

void start_ws_client() {
  ws_client->on_message = [](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message) {
    string msg_str = in_message -> string();
    cout << msg_str << endl;
    json msg_json;
    try {
      msg_json = json::parse(msg_str);
    } catch (json::parse_error& err) {
      cout << "Could not parse the following JSON, dropping message." << endl;
      cout << "\"\"\"" << endl;
      cout << msg_str << endl;
      cout << "\"\"\"" << endl;
      cout << "More information:" << endl;
      cout << "-------------------------------" << endl;
      cout << err.what() << endl;
      cout << "-------------------------------" << endl;
      return;
    }

    mtype msg_type;
    string msg_type_str;
    try {
      msg_type_str = msg_json.at("type");
      msg_type = msg_types.at(msg_type_str);
    } catch (json::out_of_range& err) {
      cout << "Message type not specified! Dropping message." << endl;
      return;
    } catch (json::type_error& err) {
      cout << "Message is not a valid JSON object! Dropping message." << endl;
    } catch (std::out_of_range& err) {
      cout << "Message type " << msg_type_str << " unknown! Dropping message." << endl;
    }

    switch (msg_type) {
      case method:
        string method_name = "unknown";
        try {
          method_name = msg_json.at("method");
          cout << "Getting arguments!" << endl;
          auto method_args = msg_json.at("args");
          auto handler = method_handlers.at(method_name);
          cout << "Calling handler!" << endl;
          handler(method_args, connection);
        } catch (json::out_of_range& err) {
          cout << "Method request does not specify method or arguments! Dropping message." << endl;
        } catch (std::out_of_range& err) {
          cout << "No handler defined for message of type " << method_name << "! Dropping message." << endl;
        }
        break;
    }
  };

  ws_client->on_open = [](std::shared_ptr<WsClient::Connection> connection) {
    cout << "Connected to server." << endl;
    connection -> send("{ \"type\": \"id\", \"id\": \"backend\" }");
  };

  ws_client->on_close = [](std::shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
    cout << "Server connection closed with status " << status << endl;
  };

  ws_client->on_error = [](std::shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    cout << "Websocket error " << ec << ": " << ec.message() << endl;
  };

  ws_client->start();
}