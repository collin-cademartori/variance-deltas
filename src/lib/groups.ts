import { SvelteMap } from "svelte/reactivity";
import { delete_group } from "./tree_methods.ts";

export const groups : Map<string, Set<string>> = new SvelteMap();

export function handle_groups(data : object) {
  for(const [gname, nnames] of Object.entries(data)) {
    if(!groups.has(gname)) {
      groups.set(gname, new Set(nnames as string[]));
    }
  }
  
  for(const gname of groups.keys()) {
    if(!Object.keys(data).includes(gname)) {
      groups.delete(gname);
    }
  }
}

export function remove_group(group_name : string) {
  delete_group({
    "group_name" : group_name
  });
}

// export function make_group(group_name : string, node_names : Array<string>) {
//   if(groups.has(group_name)) {
//     throw new Error("Group already exists with this name.");
//   } else if(group_name.length == 0) {
//     throw new Error("Group must have name.")
//   } else {
//     groups.set(group_name, new Set(node_names));
//   }
// }