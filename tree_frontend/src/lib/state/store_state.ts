import { SvelteMap } from "svelte/reactivity";
import { browser } from "$app/environment";
import { type name_t } from "./names.ts";

export function store_state(sid : string | undefined, state_prefix : string, state : boolean | string | object) {
  if(browser && sid) {
    const item_key = `${sid}-${state_prefix}`;
    if(state instanceof SvelteMap) {
      state = [...state.entries()];
    }
    if(state != undefined) {
      localStorage.setItem(item_key, JSON.stringify(state))
    } else {
      localStorage.removeItem(item_key)
    }
  }
}

export function restore_state_bool(sid : string | undefined, state_prefix : string, default_value : boolean = true) {
  if(browser && sid) {
    const str_value = localStorage.getItem(`${sid}-${state_prefix}`);
    if(str_value == null) {
      return default_value;
    }
    const restore_value = JSON.parse(str_value);
    if(typeof restore_value != "boolean") {
      return default_value;
    } else {
      return restore_value;
    }
  } else {
    return default_value;
  }
}

export function restore_state_string(sid : string | undefined, state_prefix : string, default_value : string | undefined) {
  if(browser && sid) {
    const str_value = localStorage.getItem(`${sid}-${state_prefix}`);
    if(str_value == null) {
      return default_value;
    }
    const restore_value = JSON.parse(str_value);
    if(typeof restore_value != "string") {
      return default_value;
    } else {
      return restore_value;
    }
  } else {
    return default_value;
  }
}

export function restore_state_map(sid : string | undefined, state_prefix : string, default_items : Array<[string, name_t]>) {
  if(browser && sid) {
    const str_value = localStorage.getItem(`${sid}-${state_prefix}`);
    if(str_value == null) {
      return new SvelteMap(default_items);
    }
    const restore_value = JSON.parse(str_value);
    if(Array.isArray(restore_value)) {
      return new SvelteMap<string, name_t>(restore_value);
    } else {
      return new SvelteMap(default_items);
    }
  } else {
    return new SvelteMap<string, name_t>(default_items);
  }
}

export function restore_state_groups(sid : string | undefined) : SvelteMap<string, Set<string>> {
  if(browser && sid) {
    const str_value = localStorage.getItem(`${sid}-groups`);
    if(str_value == null) {
      return new SvelteMap();
    }
    try {
      const restore_value = JSON.parse(str_value);
      // Stored as array of [name, array_of_nodes] pairs
      if(Array.isArray(restore_value)) {
        const entries : Array<[string, Set<string>]> = restore_value.map(
          ([name, nodes] : [string, string[]]) => [name, new Set(nodes)]
        );
        return new SvelteMap(entries);
      }
    } catch {
      // Invalid JSON, return empty
    }
    return new SvelteMap();
  } else {
    return new SvelteMap();
  }
}

export function store_state_groups(sid : string | undefined, groups : Map<string, Set<string>>) {
  if(browser && sid) {
    const item_key = `${sid}-groups`;
    // Convert Map<string, Set<string>> to array of [name, array] pairs
    const serializable = [...groups.entries()].map(
      ([name, nodes]) => [name, [...nodes]]
    );
    localStorage.setItem(item_key, JSON.stringify(serializable));
  }
}