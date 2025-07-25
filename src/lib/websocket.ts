import { type flat_tree } from "./test_tree.ts";

export const ws = new WebSocket("ws://localhost:8000");

const handlers = [];
const queue : string[] = [];
let connected = false;

function send_message(msg : string) {
  if(connected) {
    ws.send(msg);
  } else {
    queue.push(msg);
  }
}

export function handle_message(handler : (data : flat_tree) => void) {
  handlers.push(handler);
}

export function make_method_caller(method_name : string, arg_keys : string[]) {
  return(function(method_args : object) {
    const passed_args = Object.keys(method_args);
    for(const arg_key of arg_keys) {
      if(!passed_args.includes(arg_key)) {
        throw new Error(`Cannot call method ${method_name} without argument ${arg_key}`);
      }
    }
    send_message(JSON.stringify({
      type : "method",
      method : method_name,
      args : method_args
    }));
  });
}

ws.addEventListener("open", () => {
  console.log("Connection established with websocket server.")
  ws.send(JSON.stringify({type: "id", id: "frontend"}));
  connected = true;
  let queued_msg = null;
  while((queued_msg = queue.shift()) != null) {
    send_message(queued_msg);
  }
});

ws.addEventListener("close", () => {
  console.log("Lost connection with websocket server.")
  connected = false;
})

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