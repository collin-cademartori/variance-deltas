import type { flat_node } from "./types.ts";

export type render_config = {
  label_height: number,
  show_globals: boolean,
  draw_color: string,
  highlight_color: string,
  draw_static: boolean,
  transition_dur: number,
  format: 'long' | 'normal'
}

export type coordinates = {
  x: d3.ScaleLinear<number, number>,
  y: d3.ScaleLinear<number, number>
}

export type global_data = {
  limit : number | undefined,
  params : string[],
}

export type branch_datum = {
  child: flat_node,
  parent: flat_node
}

export type event_handlers = {
  branch_select: (d: branch_datum) => void,
  node_select: (d: flat_node) => void,
  node_hover: (d: flat_node) => void,
  node_unhover: (d: flat_node) => void
}

type partial_config = Partial<render_config>

const default_config = {
  label_height: 10,
  show_globals: true,
  draw_color: "black",
  highlight_color: "black",
  draw_static: false,
  transition_dur: 200,
  format: 'normal'
}

export function make_config(options: partial_config) {
  const def_copy = Object.assign({}, default_config);
  return(Object.assign(def_copy, options));
}