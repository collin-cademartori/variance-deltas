import { SvelteSet } from "svelte/reactivity";

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

