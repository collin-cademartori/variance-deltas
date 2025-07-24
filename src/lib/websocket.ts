import { type flat_tree } from "./test_tree.ts";

export const ws = new WebSocket("ws://localhost:8000");

const handlers = [];

export function handle_message(handler : (data : flat_tree) => void) {
  handlers.push(handler);
}

ws.addEventListener("open", () => {
  console.log("Connection established with websocket server.")
  ws.send(JSON.stringify({type: "id", id: "frontend"}));
});

ws.addEventListener("message", (event) => {
  if(event.data === "test_message") {
    ws.send("test_receipt");
    return;
  }
  try {
      const pdata = JSON.parse(event.data);
      switch(pdata.type) {
        case "tree":
          handlers.forEach((h) => h(JSON.parse(pdata.tree)));
          break;
        default:
          console.error("Received of unknown type! ", pdata);
      }
    } catch (err) {
      console.error("Could not convert message to JSON:\n ", event.data);
      console.error(err);
    }
});