#include <simple-websocket-server/client_ws.hpp>
#include <ws_client.hpp>

using namespace std;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

WsClient ws_client("localhost:8000");

void start_ws_client(string initial_message) {
  ws_client.on_message = [](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message) {
    if(in_message -> string() == "test_message") {
      connection -> send("test_receipt");
    }
  };

  ws_client.on_open = [&initial_message](std::shared_ptr<WsClient::Connection> connection) {
    cout << "Connected to server, sending tree." << endl;
    connection -> send("{ \"type\": \"id\", \"id\": \"backend\" }");
    connection -> send(initial_message);
  };

  ws_client.on_close = [](std::shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
    cout << "Server connection closed with status " << status << endl;
  };

  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  ws_client.on_error = [](std::shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    cout << "Websocket error " << ec << ": " << ec.message() << endl;
  };

  ws_client.start();
}
