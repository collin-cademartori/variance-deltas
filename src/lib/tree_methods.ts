import { make_method_caller } from "./websocket.ts";

export const get_tree = make_method_caller("get_tree", []);
export const divide_branch = make_method_caller("divide_branch", ["params_kept"]);