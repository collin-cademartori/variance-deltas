#include <simple-websocket-server/client_ws.hpp>

using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using namespace std;

int main(int, char* []) {

  WsClient client("localhost:8000");
  client.on_message = [](shared_ptr<WsClient::Connection> connection, shared_ptr<WsClient::InMessage> in_message) {
    if(in_message -> string() == "test_message") {
      connection -> send("test_receipt");
    }
  };

  client.on_open = [](shared_ptr<WsClient::Connection> connection) {
    cout << "Connected to server, awaiting messages." << endl;
  };

  client.on_close = [](shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
    cout << "Server connection closed with status " << status << endl;
  };

  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  client.on_error = [](shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    cout << "Websocket error " << ec << ": " << ec.message() << endl;
  };

  client.start();

}