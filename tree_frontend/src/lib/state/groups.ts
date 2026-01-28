import type { HierarchyNode } from "d3";
import type { flat_node } from "./types.ts";
import { restore_state_groups, store_state_groups } from "./store_state.ts";

// TODO: Replace with real session ID when session saving is implemented.
// Currently all sessions share the same groups in localStorage.
const session_id = "1234";

// Initialize groups from localStorage
export const groups : Map<string, Set<string>> = restore_state_groups(session_id);

function save_groups() {
  store_state_groups(session_id, groups);
}

export function make_group(group_name : string, node_names : Array<string>) {
  if(groups.has(group_name)) {
    throw new Error("Group already exists with this name.");
  } else if(group_name.length == 0) {
    throw new Error("Group must have name.");
  } else {
    groups.set(group_name, new Set(node_names));
    save_groups();
  }
}

export function remove_group(group_name : string) {
  groups.delete(group_name);
  save_groups();
}

// Remove nodes from groups that no longer exist in the tree.
// Deletes groups that become empty.
export function prune_groups(valid_node_names : Set<string>) {
  let changed = false;
  const groups_to_delete : string[] = [];

  for(const [group_name, node_set] of groups.entries()) {
    for(const node_name of node_set) {
      if(!valid_node_names.has(node_name)) {
        node_set.delete(node_name);
        changed = true;
      }
    }
    if(node_set.size === 0) {
      groups_to_delete.push(group_name);
    }
  }

  for(const group_name of groups_to_delete) {
    groups.delete(group_name);
    changed = true;
  }

  if(changed) {
    save_groups();
  }
}

export function expand_groups(tree: HierarchyNode<flat_node>) {
  let changed = false;

  for (const [_group_name, node_set] of groups.entries()) {
    const nodes_to_add = new Set<string>();

    // For each node in the group, find all ancestors
    for (const node_name of node_set) {
      // Find this node in the tree
      const node = tree.descendants().find(n => n.data.name === node_name);
      if (!node) continue; // Node was deleted (should already be pruned)

      // Walk up to root, adding all ancestors
      let ancestor = node.parent;
      while (ancestor) {
        const ancestor_name = ancestor.data.name;
        if (!node_set.has(ancestor_name)) {
          nodes_to_add.add(ancestor_name);
          changed = true;
        }
        ancestor = ancestor.parent;
      }
    }

    // Add all discovered ancestors
    for (const name of nodes_to_add) {
      node_set.add(name);
    }
  }

  if (changed) {
    save_groups();
  }
}

