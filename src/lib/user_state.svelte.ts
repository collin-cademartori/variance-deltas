import { clear_branches, clear_selection } from "./selection.svelte.ts";
import { reset_styles } from "./draw_tree.ts";
import { stratify, type HierarchyNode, type ScaleLinear } from "d3";
import { type flat_node, type flat_tree, type flat_branch } from "./tree.ts";
import { type label_data_t, type branch_data_t, selector } from "./selection.svelte.ts";
import { groups } from "./groups.ts";
import { annotate_tree } from "./tree.ts";
import { draw_tree } from "./draw_tree.ts";
import { E } from "../../.svelte-kit/output/server/chunks/index.js";

type user_state_t = 'base' | 'extruding' | 'dividing' | 'auto-dividing' | 'merging' | 'auto-merging' | 'groups' | 'add-group';

// type node_group_t = {
//   name : string,
//   node_names : string[]
// } | undefined

type state_t = {
  state: user_state_t,
  tree: HierarchyNode<flat_node> | undefined,
  group: string | undefined
};

type numeric_scale = ScaleLinear<number, number, never>;

type node_handler = (d : flat_node) => label_data_t;
type branch_handler = (d : flat_branch) => object; // change to branch_data_t

let _user_state : user_state_t = $state('base');
let _tree : HierarchyNode<flat_node> | undefined;
let _group : string | undefined = $state(undefined);
let _create_tree : (data : flat_tree) => void;

export const user_state : state_t = $state({ 
  get state() {
    return(_user_state);
  },
  set state(st: user_state_t) {
    clear_selection();
    clear_branches();
    reset_styles();
    if(st === "add-group") {
      selector.type = "anc";
    } else {
      selector.type = "node";
    }
    _user_state = st;
  },
  get tree() {
    return(_tree);
  },
  set tree(tree : HierarchyNode<flat_node>) {
    _create_tree([...tree].map((n) => n.data));
    _tree = tree;
  },
  get group() {
    return(_group);
  },
  set group(group : string | undefined) {
    _group = group;
    _create_tree([..._tree].map((n) => n.data));
  }
});

export function setup_tree(x: numeric_scale, y: numeric_scale, l_height : number, nh : node_handler, bh : branch_handler) {
  _create_tree = function(data : flat_tree) {
    _tree = (stratify<flat_node>()
              .id((n : flat_node) => n.name.toString())
              .parentId((n : flat_node) => n.parent.toString()))(data);
    const fil_data = _group == undefined ? data : data.filter((node) => groups.get(user_state.group)?.has(node.name));
    if(fil_data.length == 0) {
      console.warn("NO DATA")
    }
    const ann_tree = annotate_tree(
      fil_data, y.invert(l_height), x, y
    );
    const ft : flat_tree = [...ann_tree].map((n) => n.data);
    draw_tree(
      ft, x, y, l_height,
      nh, bh,
      document.styleSheets[0]
    );
  }
}
