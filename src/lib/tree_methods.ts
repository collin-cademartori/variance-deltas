import { make_method_caller } from "./websocket.ts";

export const get_tree = make_method_caller("get_tree", []);
export const divide_branch = make_method_caller("divide_branch", ["node_name", "params_kept"]);
export const extrude_branch = make_method_caller("extrude_branch", ["node_name", "params_kept"]);
export const reset_tree = make_method_caller("reset_tree", []);