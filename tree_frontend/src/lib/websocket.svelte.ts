import { type flat_tree } from "./state/types.ts";
import { browser } from "$app/environment";

// Reactive connection state
let _connected = $state(false);
let _busy = $state(false);

export const connection = {
  get connected() { return _connected; },
  get busy() { return _busy; },
  get frozen() { return _busy || !_connected; }
};

// Connect to the same host/port that served the page
const wsUrl = browser ? `ws://${window.location.host}` : 'ws://localhost:8000';
export const ws = browser ? new WebSocket(wsUrl) : null;

type tree_handler_t = (tree_data : flat_tree, globals_data : string[], global_limit : number, groups_data : object) => void;

const tree_handlers : tree_handler_t[] = [];
const queue : string[] = [];

function send_message(msg : string) {
  if(_connected && ws) {
    ws.send(msg);
  } else {
    queue.push(msg);
  }
}

export function handle_message(handler : tree_handler_t) {
  tree_handlers.push(handler);
}

export function make_method_caller(method_name : string, arg_keys : string[]) {
  return(function(method_args : object) {
    const passed_args = Object.keys(method_args);
    for(const arg_key of arg_keys) {
      if(!passed_args.includes(arg_key)) {
        throw new Error(`Cannot call method ${method_name} without argument ${arg_key}`);
      }
    }
    _busy = true;
    send_message(JSON.stringify({
      type : "method",
      method : method_name,
      args : method_args
    }));
  });
}

if (browser && ws) {
  ws.addEventListener("open", () => {
    console.log("Connection established with websocket server.")
    ws.send(JSON.stringify({type: "id", id: "frontend"}));
    _connected = true;
    let queued_msg = null;
    while((queued_msg = queue.shift()) != null) {
      send_message(queued_msg);
    }
  });

  ws.addEventListener("close", () => {
    console.log("Lost connection with websocket server.")
    _connected = false;
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
          tree_handlers.forEach((h) => h(JSON.parse(pdata.tree), JSON.parse(pdata.globals), JSON.parse(pdata.global_limit), JSON.parse(pdata.groups)));
          _busy = false;
          break;
        default:
          console.error("Received message of unknown type: ", pdata);
      }
    } catch (err) {
      console.error("Could not convert message to JSON:\n ", event.data);
      console.error(err);
      _busy = false;
    }
  });
}
