import { serveFile, serveDir } from "jsr:@std/http/file-server";
import { parseArgs } from "jsr:@std/cli@^1.0.20/parse-args";
import * as path from "jsr:@std/path";

// Get the directory where this executable is located
const execPath = Deno.realPathSync(Deno.execPath());
const execDir = path.dirname(execPath);

// Construct paths relative to executable location
const graph_test_path = path.join(execDir, "graph_test");
const site_root = path.join(execDir, "client");

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

const args = parseArgs(Deno.args, {
  string: ["M", "D", "S", "N", "port"],
  default: {
    port: "8765"
  }
});

if(args.M == null || args.D == null || args.S == null || args.N == null) {
  console.error("All arguments mandatory.")
  Deno.exit(1);
}

const PORT = parseInt(args.port, 10);
if (isNaN(PORT) || PORT < 1 || PORT > 65535) {
  console.error("Invalid port number. Must be between 1 and 65535.");
  Deno.exit(1);
}

console.log(`Starting server on port ${PORT}...`);
Deno.serve({ port: PORT }, (req) => {

  if (req.headers.get("upgrade") === "websocket") {
    const res = handle_ws_request(req);
    return res;
  } else {
    const res = handle_http_request(req);
    return res;
  }
});

console.log("Starting client subprocess.");
try {
  const command = new Deno.Command(graph_test_path,
    {
      args: [
        "-M", args.M, "-D", args.D, "-S", args.S, "-N", args.N, "-P", PORT.toString()
      ],
      stdout: "inherit",
      stderr: "inherit"
    }
  );
  const _subproc = command.spawn();

  // Wait for the subprocess to complete in the background
  _subproc.status.then((status) => {
    if (!status.success) {
      console.error(`Backend process exited with code: ${status.code}`);
    }
    console.log("Backend process terminated. Shutting down server.");
    Deno.exit(status.success ? 0 : 1);
  });
} catch (err) {
  console.error("Failed to launch backend: ", err);
}

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
  } else if (pathname.startsWith("/")) {
    return serveFile(req, site_root + pathname);
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
            handle_tree(pdata);
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

    // Handle disconnection based on client type
    if (id === "backend") {
      socket.addEventListener("close", () => {
        console.log("Backend WebSocket disconnected. Shutting down server.");
        Deno.exit(1);
      });
      open_browser();
    } else if (id === "frontend") {
      socket.addEventListener("close", () => {
        console.log("Frontend disconnected.");
        clients.frontend = undefined;
        console.log(`Reconnect at: http://localhost:${PORT}/`);
      });
    }

    let queued_msg = null;
    while((queued_msg = queues[id].shift()) != null) {
      socket.send(queued_msg);
    }
  } else {
    console.error("Client id not recognized: ", id);
    socket.close();
  }
}

async function open_browser() {
  const url = `http://localhost:${PORT}`;
  console.log(`Opening browser to ${url}...`);

  try {
    let command: string;
    let args: string[];

    switch (Deno.build.os) {
      case "darwin":  // macOS
        command = "open";
        args = [url];
        break;
      case "linux":
        command = "xdg-open";
        args = [url];
        break;
      case "windows":
        command = "cmd";
        args = ["/c", "start", url];
        break;
      default:
        console.warn(`Unknown OS: ${Deno.build.os}, cannot open browser`);
        return;
    }

    const cmd = new Deno.Command(command, { args });
    await cmd.output();
  } catch (err) {
    console.error("Failed to open browser:", err);
    console.log(`Please open your browser manually to: http://localhost:${PORT}`);
  }
}

function try_send(client_name : "frontend" | "backend", message : string) {
  if(clients[client_name] == null) {
    queues[client_name].push(message);
  } else {
    clients[client_name].send(message);
  }
}

function handle_tree(msg_data : object) { //tree : object, globals : object, global_limit : number, groups : object
  const msg_data_strings = Object.fromEntries(
    Object.entries(msg_data).map(([k,v]) => [k, JSON.stringify(v)])
  );
  const tree_message = JSON.stringify(Object.assign(msg_data_strings, {
    "type" : "tree"
  }));
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