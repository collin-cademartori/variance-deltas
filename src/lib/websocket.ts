export const ws = new WebSocket("ws://localhost:8000");

ws.addEventListener("open", () => {
  console.log("Connection established with websocket server.")
});

ws.addEventListener("message", (event) => {
  if(event.data === "test_message") {
    ws.send("test_receipt");
  }
});