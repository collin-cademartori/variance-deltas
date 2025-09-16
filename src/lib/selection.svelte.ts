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
    clear_selection();
    clear_branches();
    reset_styles();
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

export const node_selection : selection_type = $state({
  main: new SvelteSet(), alt: new SvelteSet(), del: new SvelteSet()
});

export function clear_selection() {
  for(const channel of Object.values(node_selection)) channel.clear();
}

export const branch_selection : selection_branch_type = $state({
  main: new SvelteSet(), alt: new SvelteSet(), del: new SvelteSet()
});

export function clear_branches() {
  for(const channel of Object.values(branch_selection)) channel.clear();
}

