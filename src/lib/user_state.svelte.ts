import { draw_tree } from "./draw_tree.ts";
import { stratify, type HierarchyNode, type ScaleLinear } from "d3";
import { type flat_node, type flat_tree, type flat_branch } from "./tree.ts";
import { selector, selection, hover } from "./selection.svelte.ts";
import { groups } from "./groups.ts";
import { annotate_tree } from "./tree.ts";
import { type name_t, make_short, global_latex } from "./names.ts";
import { SvelteMap } from "svelte/reactivity";

type user_state_t = 'base' | 'extruding' | 'dividing' | 'auto-dividing' | 'merging' | 'auto-merging' | 'groups' | 'add-group' | 'settings' | 'deleting';

type state_t = {
  state: user_state_t,
  tree: HierarchyNode<flat_node> | undefined,
  group: string | undefined,
  globals: string[],
  global_limit: number | undefined,
  layout_format: 'long' | 'normal',
  show_globals: boolean,
  names: SvelteMap<string, name_t>
};

type numeric_scale = ScaleLinear<number, number, never>;

type node_handler = (d : flat_node) => void;
type branch_handler = (d : flat_branch) => void; // change to branch_data_t

const session_id = "1234";

function store_state(state_prefix : string, state : boolean | string | object) {
  const item_key = `${session_id}-${state_prefix}`;
  if(state instanceof SvelteMap) {
    state = [...state.entries()];
  }
  if(state != undefined) {
    localStorage.setItem(item_key, JSON.stringify(state))
  } else {
    localStorage.removeItem(item_key)
  }
}

function restore_state_bool(state_prefix : string, default_value : boolean = true) {
  const restore_value = JSON.parse(localStorage.getItem(`${session_id}-${state_prefix}`));
  if(typeof restore_value != "boolean") {
    return default_value;
  } else {
    return restore_value;
  }
}

function restore_state_string(state_prefix : string, default_value : string | undefined) {
  const restore_value = JSON.parse(localStorage.getItem(`${session_id}-${state_prefix}`));
  if(typeof restore_value != "string") {
    return default_value;
  } else {
    return restore_value;
  }
}

function restore_state_map(state_prefix : string, default_items : Array<[string, name_t]>) {
  const restore_value = JSON.parse(localStorage.getItem(`${session_id}-${state_prefix}`));
  if(Array.isArray(restore_value)) {
    return new SvelteMap(restore_value);
  } else {
    return new SvelteMap(default_items);
  }
}

let _user_state : user_state_t = $state('base');
let _names : SvelteMap<string, name_t> = $state(restore_state_map("names", [["__globals__", {
  type: 'latex',
  name: '\\bar{g}',
  formatted_name: global_latex
}]]) as SvelteMap<string, name_t>);
let _tree : HierarchyNode<flat_node> | undefined;
let _group : string | undefined = $state(restore_state_string("group", undefined));
let _layout_format : 'long' | 'normal' = $state(restore_state_string("layout_format", 'normal') as 'long' | 'normal');
let _show_globals : boolean = $state(restore_state_bool("show_globals", true));
let _create_tree : (data : flat_tree) => void;

export const user_state : state_t = $state({ 
  get state() {
    return(_user_state);
  },
  set state(st: user_state_t) {
    selection.clear();
    hover.node = [];
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
    selection.clear();
    _create_tree([...tree].map((n) => n.data));
    _tree = tree;
  },
  get group() {
    return(_group);
  },
  set group(group : string | undefined) {
    _group = group;
    store_state("group", group);
    selection.clear();
    _create_tree([..._tree].map((n) => n.data));
  },
  globals: [],
  global_limit: undefined,
  get layout_format() {
    return(_layout_format);
  },
  set layout_format(fmt : 'long' | 'normal') {
    _layout_format = fmt;
    store_state("layout_format", fmt)
    _create_tree([..._tree].map((n) => n.data));
  },
  get show_globals() {
    return(_show_globals);
  },
  set show_globals(show: boolean) {
    _show_globals = show;
    store_state("show_globals", show);
    _create_tree([..._tree].map((n) => n.data));
  },
  get names() {
    return(_names);
  },
  set names(new_names : SvelteMap<string, name_t>) {
    _names = new_names;
    store_state("names", new_names)
    _create_tree([..._tree].map((n) => n.data));
  }
});

export function update_names(names: Set<string>) {
  names.forEach((name) => {
    if(!_names.has(name)) {
      _names.set(name, make_short(name));
    }
  })
}

export function setup_tree(x: numeric_scale, y: numeric_scale, l_height : number) {
  _create_tree = function(data : flat_tree) {
    _tree = (stratify<flat_node>()
              .id((n : flat_node) => n.name.toString())
              .parentId((n : flat_node) => n.parent.toString()))(data);
    const fil_data = _group == undefined ? data : data.filter((node) => groups.get(user_state.group)?.has(node.name));
    if(fil_data.length == 0) {
      console.warn("NO DATA")
    }
    const ann_tree = annotate_tree(
      fil_data, user_state.names, l_height, x, y, user_state.globals, user_state.layout_format, user_state.show_globals
    );
    const ft : flat_tree = [...ann_tree].map((n) => n.data);
    draw_tree(
      ft, x, y, l_height, user_state.global_limit, user_state.globals, user_state.show_globals,
      document.styleSheets[0]
    );
    // draw_geometry(
    //   ft, "tree_g", x, y,
    //   () => {return({})}, document.styleSheets[0],
    //   "blue", true, "_selection"
    // );
  }
}
