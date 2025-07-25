#include <nlohmann/json.hpp>

void start_ws_client();
void handle_method(std::string method_name, std::function<std::optional<std::string>(nlohmann::json)> handler);