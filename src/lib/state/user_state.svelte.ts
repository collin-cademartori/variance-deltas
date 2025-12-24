import { draw_tree, draw_highlight, draw_geometry } from "./draw_tree.ts";
import { stratify, type HierarchyNode, } from "d3";
import type { flat_node, flat_tree } from "./types.ts";
import { selector, selection, hover, selection_channels } from "./selection.svelte.ts";
import { groups } from "./groups.ts";
import { annotate_tree } from "./tree.ts";
import { type name_t, make_short, global_latex } from "./names.ts";
import { SvelteMap } from "svelte/reactivity";
import { type branch_datum, type coordinates, make_config } from "./draw_data.ts";
import { store_state, restore_state_bool, restore_state_map, restore_state_string } from "./store_state.ts";

type user_state_t = 'base' | 'extruding' | 'dividing' | 'auto-dividing' | 'merging' | 'auto-merging' | 'groups' | 'add-group' | 'settings' | 'deleting';

type state_t = {
  state: user_state_t,
  tree: HierarchyNode<flat_node> | undefined,
  group: string | undefined,
  globals: string[],
  global_limit: number | undefined,
  layout_format: 'long' | 'normal',
  show_globals: boolean,
  names: SvelteMap<string, name_t>,
  svg_height: number
};

const session_id = "1234";

let _user_state : user_state_t = $state('base');
let _names : SvelteMap<string, name_t> = $state(restore_state_map(session_id, "names", [["__globals__", {
  type: 'latex',
  name: '\\bar{g}',
  formatted_name: global_latex
}]]) as SvelteMap<string, name_t>);
let _tree : HierarchyNode<flat_node> | undefined;
let _group : string | undefined = $state(restore_state_string(session_id, "group", undefined));
let _layout_format : 'long' | 'normal' = $state(restore_state_string(session_id, "layout_format", 'normal') as 'long' | 'normal');
let _show_globals : boolean = $state(restore_state_bool(session_id, "show_globals", true));
let _svg_height = $state(0);
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
    store_state(session_id, "group", group);
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
    store_state(session_id, "layout_format", fmt)
    _create_tree([..._tree].map((n) => n.data));
  },
  get show_globals() {
    return(_show_globals);
  },
  set show_globals(show: boolean) {
    _show_globals = show;
    store_state(session_id, "show_globals", show);
    _create_tree([..._tree].map((n) => n.data));
  },
  get names() {
    return(_names);
  },
  set names(new_names : SvelteMap<string, name_t>) {
    _names = new_names;
    store_state(session_id, "names", new_names)
    _create_tree([..._tree].map((n) => n.data));
  },
  get svg_height() {
    return(_svg_height)
  }
});

export function update_names(names: Set<string>) {
  names.forEach((name) => {
    if(!_names.has(name)) {
      _names.set(name, make_short(name));
    }
  })
}

const handlers = {
  branch_select: (d: branch_datum) => {
    if(user_state.state === 'dividing' || user_state.state === 'auto-dividing') {
      if(selection.has(d.parent.name, "main") && selection.has(d.child.name, "main")) {
        selection.clear("main");
      } else {
        selection.set_nodes([d.child.name, d.parent.name], "main");
      }
    } 
  },
  node_select: (d: flat_node) => {
    if(user_state.state === 'extruding') {
      if(selection.has(d.name, "main")) {
        selection.clear("main");
      } else {
        selection.set_nodes([d.name], "main");
      }
    } else if (user_state.state === 'deleting') {
      if(selection.has(d.name, "main")) {
        selection.clear("main");
      } else {
        selection.set_nodes([d.name], "main");
      }
    } else if (user_state.state === 'merging') {
      if(selection.has(d.name, "main")) {
        selection.clear();
      } else if (selection.has(d.name, "alt")) {
        selection.clear("alt");
      } else if (selection.size("main") > 0) {
        selection.set_nodes([d.name], "alt");
      } else {
        selection.set_nodes([d.name], "main")
      }
    } else if (user_state.state === 'add-group') {
      const sel_node = user_state.tree?.find((node) => node.data.name === d.name);
      let sel_names : string[] = [];
      if(selector.type === "anc") {
        if(sel_node?.data.name && selection.has(sel_node?.data.name, "main")) {
          sel_names = sel_node?.descendants().map((desc) => desc.data.name);
          sel_names.forEach((node_name) => selection.delete(node_name, "main"));
        } else {
          sel_names = sel_node?.ancestors().map((anc) => anc.data.name) ?? [];
          sel_names.forEach((name) => selection.add(name, "main"));
        }
      } else if(selector.type === "desc") {
        if(sel_node?.data.name && selection.has(sel_node?.data.name, "del")) {
          sel_names = sel_node?.ancestors().map((anc) => anc.data.name);
          sel_names.forEach((node_name) => selection.delete(node_name, "del"));
        } else {
          sel_names = sel_node?.descendants().map((desc) => desc.data.name) ?? [];
          sel_names.forEach((name) => selection.add(name, "del"));
        }
      } 
    } 
  },
  node_hover: (d: flat_node) => {
    if(user_state.state === "base") {
      hover.node = [d.name];
    }
  },
  node_unhover: () => {
    hover.node = [];
  }
}

export function setup_tree(coord: coordinates, l_height : number) {
  _create_tree = function(data : flat_tree) {
    _tree = (stratify<flat_node>()
              .id((n : flat_node) => n.name.toString())
              .parentId((n : flat_node) => n.parent.toString()))(data);
    const fil_data = _group == undefined ? data : data.filter((node) => groups.get(user_state.group)?.has(node.name));
    const global_data = {
      limit: user_state.global_limit,
      params: user_state.globals
    }
    const render_config = make_config({
      label_height: l_height,
      show_globals: user_state.show_globals,
      format: user_state.layout_format
    });
    const ann_tree = annotate_tree(fil_data, user_state.names, coord, global_data, render_config);
    const ft : flat_tree = [...ann_tree].map((n) => n.data);
    _svg_height = draw_tree(ft, coord, global_data, render_config, handlers, user_state.names);

    hover.on_change(() => {
      const filt_tree = data.filter((node) => hover.node.includes(node.name));
      draw_highlight(filt_tree, coord.x, render_config, user_state.svg_height);
    });

    if(!render_config.draw_static){   
      selection?.on_change(() => {
        for(const channel of selection_channels) {
          const ft = data.filter((node) => selection.has(node.name, channel));
          draw_geometry(
            ft, "tree_g", coord,
            render_config, "_" + channel, 1.15, handlers
          );
        }
        // Clear all selection classes
        for(const channel of selection_channels) {
          const cn = `${channel}_label_selected`;
          const els = Array.from(document.getElementsByClassName(cn));
          for(let ei = 0; ei < els.length; ++ei) {
            els[ei].classList.remove(cn);
          }
        }
        // Add selection classes for newly selection items
        for(const channel of selection_channels) {
          const cn = `${channel}_label_selected`;
          const sel_nodes = selection.nodes(channel);
          for(let ni = 0; ni < selection.size(channel); ++ni) {
            document.getElementById(`${sel_nodes[ni]}_div`).classList.add(cn);
          }
        }
      });
    }
  }
}
