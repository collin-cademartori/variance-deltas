#include <nlohmann/json.hpp>

void initialize_ws_client(const std::string& host, int port);
void start_ws_client();
void handle_method(std::string method_name, std::function<std::optional<std::string>(nlohmann::json)> handler);