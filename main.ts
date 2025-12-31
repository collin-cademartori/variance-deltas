import { serveFile, serveDir } from "jsr:@std/http/file-server";

const site_root = Deno.build.standalone ? (import.meta.dirname + "/client") : "../MRF_Client/build";

type WSData = {
  id? : 'backend' | 'frontend';
}

interface WebSocketWithData extends WebSocket {
  data : WSData
}

type Clients = { 
  backend : undefined | WebSocketWithData,
  frontend : undefined | WebSocketWithData 
}

type Queues = { 
  backend : string[],
  frontend : string[]
}

function add_data_to_ws (ws : WebSocket) : asserts ws is WebSocketWithData {
  Object.assign(ws, {data : {}});
}

const clients : Clients = {
  backend: undefined,
  frontend: undefined
};

const queues : Queues = {
  frontend: [],
  backend: []
};

console.log("Starting server.");
Deno.serve((req) => {

  if (req.headers.get("upgrade") === "websocket") {
    const res = handle_ws_request(req);
    return res;
  } else {
    const res = handle_http_request(req);
    return res;
  }
});

// console.log("Starting client subprocess.");
// try {
//   // const command = new Deno.Command("./ws_test");
//   const command = new Deno.Command("./graph_test");
//   const _subproc = await command.output();
// } catch (err) {
//   console.error("Failed to launch backend: ", err);
// }

function handle_ws_request(req : Request) {
  const { socket, response } = Deno.upgradeWebSocket(req);

  socket.addEventListener("open", () => {
    console.log("Client connected.");
    add_data_to_ws(socket);
    socket.addEventListener("message", create_message_handler(socket));
    setTimeout(() => {
      if (socket.data.id == null) {
        console.warn("Closing connection with nonidentified client.");
        socket.close();
      }
    }, 3000);
  });

  socket.addEventListener("close", () => {
    console.log("Client closed the connection.");
  });

  socket.addEventListener("error", (err) => {
    if(err instanceof ErrorEvent) {
      console.error("Websocket error: ", err.message);
    } else {
      console.error("Websocket error: unknown.");
    }
  });

  return(response);
}

function handle_http_request(req : Request) {
  const pathname = new URL(req.url).pathname;

  if (pathname === "/") {
    return serveFile(req, site_root + "/index.html");
  } else if (pathname.startsWith("/_app")) {
    return serveDir(req, {
      fsRoot: site_root,
    });
  } else {
    console.warn("Denying request for ", pathname);
    return new Response("404: Not Found", {
      status: 404,
    });
  }
}

function create_message_handler(socket : WebSocketWithData) {
  // deno-lint-ignore no-explicit-any
  return (event : MessageEvent<any>) => {
    if (event.data === "ping") {
      socket.send("pong");
    } else {
      try {
        const pdata = JSON.parse(event.data);
        switch(pdata.type) {
          case "id":
            attach_id(socket, pdata.id);
            break;
          case "method":
            console.log("Requesting method from backend...")
            try_send("backend", event.data);
            break;
          case "tree":
            handle_tree(pdata.tree, pdata.globals, pdata.global_limit, pdata.groups);
            break;
          case "groups":
            handle_groups(pdata.groups);
            break;
          default:
            console.log("Received message! ", pdata);
        }
      } catch (_err) {
        console.error("Could not convert message to JSON:\n ", event.data);
      }
    }
  }
}

function attach_id(socket : WebSocketWithData, id : string) {
  if (id === "backend" || id === "frontend") {
    console.log("Connected to " + id + ".");
    socket.data.id = id;
    clients[id] = socket;

    let queued_msg = null;
    while((queued_msg = queues[id].shift()) != null) {
      socket.send(queued_msg);
    }
  } else {
    console.error("Client id not recognized: ", id);
    socket.close();
  }
}

function try_send(client_name : "frontend" | "backend", message : string) {
  if(clients[client_name] == null) {
    queues[client_name].push(message);
  } else {
    clients[client_name].send(message);
  }
}

function handle_tree(tree : object, globals : object, global_limit : number, groups : object) {
  const tree_message = JSON.stringify({
    "type" : "tree",
    "tree" : JSON.stringify(tree),
    "globals" : JSON.stringify(globals),
    "global_limit" : JSON.stringify(global_limit),
    "groups" : JSON.stringify(groups)
  });
  try_send("frontend", tree_message);
}

function handle_groups(groups : object) {
  const groups_message = JSON.stringify({
    "type" : "groups",
    "groups" : JSON.stringify(groups)
  });
  try_send("frontend", groups_message);
}

// function handle_method(method_data : any) {
//   const mname : string = method_data.name;
//   const margs : string[] = method_data.args;
//   clients.backend?.send(mname + ":" + margs.join(","))
// }