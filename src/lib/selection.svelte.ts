import { SvelteSet } from "svelte/reactivity";
import { reset_styles } from "./draw_tree.ts";

type selector_t = {
  type: 'node' | 'anc' | 'desc';
}

let _sel_type : selector_t["type"] = $state('node');

export const selector = $state({
  get type() {
    return(_sel_type);
  },
  set type(st : selector_t["type"]) {
    // clear_selection();
    // clear_branches();
    //reset_styles();
    selection.clear();
    _sel_type = st;
  }
});

export const selection_channels = ['main', 'alt', 'del'] as const;

export type channel_t = typeof selection_channels[number];

export type label_data_t = {
  target: string[]
  channel: channel_t
} | null

export type branch_data_t = {
  target: string[]
  channel: channel_t
} | null

export type selection_type = {
  [key in channel_t] : SvelteSet<string>
}

export type selection_branch_type = {
  [key in channel_t] : SvelteSet<[string, string]>
}

let _node_handler : () => void = () => {};

const _node_selection : selection_type = $state({
  main: new SvelteSet(), alt: new SvelteSet(), del: new SvelteSet()
});

export const selection = {
  clear: function(channel? : channel_t) {
    if(channel == null) {
      for(const channel of Object.values(_node_selection)) channel.clear();
    } else {
      _node_selection[channel].clear();
    }
    _node_handler();
  },
  size: function(channel : channel_t) {
    return(_node_selection[channel].size);
  },
  has: function(name : string, channel : channel_t) {
    return(_node_selection[channel].has(name));
  },
  add: function(name : string, channel : channel_t) {
    _node_selection[channel].add(name);
    _node_handler();
  },
  delete: function(name : string, channel : channel_t) {
    _node_selection[channel].delete(name);
    _node_handler();
  },
  nodes: function(channel : channel_t) {
    return([..._node_selection[channel]]);
  },
  set_nodes: function(names : string[], channel : channel_t) {
    _node_selection[channel].clear();
    names.map((name) => _node_selection[channel].add(name));
    _node_handler();
  },
  on_change: function(handler : () => void) {
    _node_handler = handler;
  }
}

export function clear_selection() {
  for(const channel of Object.values(_node_selection)) channel.clear();
}

const _branch_selection : selection_branch_type = $state({
  main: new SvelteSet(), alt: new SvelteSet(), del: new SvelteSet()
});

export function clear_branches() {
  for(const channel of Object.values(_branch_selection)) channel.clear();
}

