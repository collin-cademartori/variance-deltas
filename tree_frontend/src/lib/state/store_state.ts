import { SvelteMap } from "svelte/reactivity";
import { browser } from "$app/environment";
import { type name_t } from "./names.ts";

export function store_state(sid : string, state_prefix : string, state : boolean | string | object) {
  if(browser) {
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

export function restore_state_bool(sid : string, state_prefix : string, default_value : boolean = true) {
  if(browser) {
    const restore_value = JSON.parse(localStorage.getItem(`${sid}-${state_prefix}`));
    if(typeof restore_value != "boolean") {
      return default_value;
    } else {
      return restore_value;
    }
  }
}

export function restore_state_string(sid : string, state_prefix : string, default_value : string | undefined) {
  if(browser) {
    const restore_value = JSON.parse(localStorage.getItem(`${sid}-${state_prefix}`));
    if(typeof restore_value != "string") {
      return default_value;
    } else {
      return restore_value;
    }
  } else {
    return "";
  }
}

export function restore_state_map(sid : string, state_prefix : string, default_items : Array<[string, name_t]>) {
  if(browser) {
    const restore_value = JSON.parse(localStorage.getItem(`${sid}-${state_prefix}`));
    if(Array.isArray(restore_value)) {
      return new SvelteMap(restore_value);
    } else {
      return new SvelteMap(default_items);
    }
  } else {
    return new SvelteMap();
  }
}