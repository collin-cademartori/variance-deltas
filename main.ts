import { serveFile, serveDir } from "jsr:@std/http/file-server";

const site_root = "../MRF_Client/build";

console.log("Starting websocket server.");
Deno.serve((req) => {

  if (req.headers.get("upgrade") === "websocket") {
    const { socket, response } = Deno.upgradeWebSocket(req);

    socket.addEventListener("open", () => {
      console.log("Client connected, sending test message.");
      socket.send("test_message");
    });

    socket.addEventListener("message", (event) => {
      if (event.data === "ping") {
        socket.send("pong");
      } else if (event.data === "test_receipt") {
        console.log("Client sent receipt.");
      }
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

    return response;
  } else {
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
});

// console.log("Starting client subprocess.");
// const command = new Deno.Command("./ws_test");
// const subproc = await command.output();