import { make_method_caller } from "./websocket.ts";

export const get_tree = make_method_caller("get_tree", []);
export const divide_branch = make_method_caller("divide_branch", ["node_name", "params_kept"]);
export const auto_divide = make_method_caller("auto_divide", ["node_name"]);
export const extrude_branch = make_method_caller("extrude_branch", ["node_name", "params_kept"]);
export const merge_nodes = make_method_caller("merge_nodes", ["node_name", "alt_node_name"]);
export const auto_merge = make_method_caller("auto_merge", []);
export const reset_tree = make_method_caller("reset_tree", []);
export const define_group = make_method_caller("define_group", ["group_name", "node_names"]);
export const delete_group = make_method_caller("delete_group", ["group_name"]);