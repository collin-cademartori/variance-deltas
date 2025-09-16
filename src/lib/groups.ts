import { SvelteMap } from "svelte/reactivity";

export const groups : Map<string, Set<string>> = new SvelteMap();

export function make_group(group_name : string, node_names : Array<string>) {
  if(groups.has(group_name)) {
    throw new Error("Group already exists with this name.");
  } else if(group_name.length == 0) {
    throw new Error("Group must have name.")
  } else {
    groups.set(group_name, new Set(node_names));
  }
}