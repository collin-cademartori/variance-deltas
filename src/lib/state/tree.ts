import * as d3 from "d3";
import { type name_t } from "./names.ts";
import { short_name } from "./short_names.ts";
import { SvelteMap } from "svelte/reactivity";
import { consts } from "./draw_data.ts";

import type { flat_node, flat_tree, tree_node } from "./types.ts";
import type { coordinates, global_data, render_config } from "./draw_data.ts";

export function annotate_tree(
  ft : flat_tree,
  names: SvelteMap<string, name_t>,
  coord: coordinates,
  global: global_data,
  config: render_config
) {
  for(const node of ft) {
    const params = global.params == null ? node.params : [...node.params].filter((p) => !global.params.includes(p));
    if([...node.params].some((p) => !global.params.includes(p)) && config.show_globals) {  
      params.push("__globals__");
    }
    node.param_names = short_name(params, names);
    node.shortname = node.param_names.join(", ");
  }
  const tree_with_xs = compute_xs(ft, coord.y, config.format, false, config.label_height + 8);
  const label_height_y = coord.y.invert(config.label_height);
  compute_label_pos(tree_with_xs, label_height_y, coord.y, 1, config.format);
  return(tree_with_xs);
}

function compute_xs(
  ft : flat_tree,
  y_scale : d3.ScaleLinear<number, number, never>,
  layout_format : 'normal' | 'long' = 'long',
  compact : boolean = false,
  label_size : number
) {
  const test_tree = (d3.stratify<flat_node>()
                    .id((n : flat_node) => n.name.toString())
                    .parentId((n : flat_node) => n.parent.toString()))(ft);

  for(const node of test_tree) {
    node.data.sortname = node.ancestors()
      .map((n) => n.data.shortname)
      .reverse()
      .reduce((p : string, n) => p + n, "");
  }

  const leaf_sort = (l1 : tree_node, l2 : tree_node) => {
    if(l1.data.sortname == null) {
      return(-1);
    } else if(l2.data.sortname == null) {
      return(1);
    } else {
      return(l1.data.sortname > l2.data.sortname ? 1 : -1);
    }
  }

  if(layout_format === 'long') {
    const num_nodes = test_tree.descendants().length;
    for(const [nindex, node] of test_tree.descendants().sort(leaf_sort).entries()) {
      console.warn(`Can't compress past ${y_scale.invert(label_size)}, but fitting height is ${(0.9 / (num_nodes - 1))}`)
      node.data.x_pos = 0.05 + nindex * 1.1 * y_scale.invert(label_size); //Math.max((0.9 / (num_nodes - 1)), y_scale.invert(44));
      if(node.data.depth == null) {
        node.data.depth = node.ancestors().length;
      }
    } 
  } else {
    const spacing_factor = compact ? 1.2 : 2.2;
    const num_leaves = test_tree.leaves().length;
    console.log(`Computing positions of ${num_leaves} leaves`);
    for(const [lindex, leaf] of test_tree.leaves().sort(leaf_sort).entries()) {
      // leaf.data.x_pos = 0.05 + 0.9 * ((lindex + 0.1) / ((num_leaves - 1) + 0.2));
      leaf.data.x_pos = (1.1 * y_scale.invert(label_size)) + lindex * spacing_factor * y_scale.invert(label_size);
    } 

    test_tree.eachAfter((d) => {
      if(d.children != null) {
        const children = d.children;
        const x_tot = children.reduce((p, n) => 
          p + (n.data.x_pos as number),
        0);
        d.data.x_pos = x_tot / children.length;
      }

      if(d.data.depth == null) {
        d.data.depth = d.ancestors().length;
      }
    });
  }

  return(test_tree)
}

// function intersect(x1 : [number, number], x2: [number, number], fudge : number) {
//   if((x2[0] + x2[1]) < x1[0] - fudge || x2[0] > (x1[0] + x1[1]) + fudge) {
//     return(false);
//   } else {
//     return(true);
//   }
// }

// function intersect_area(c1: coord, c2: coord) {
//   if(c1.x[0] > c2.x[0] + c2.x[1] || c2.x[0] > c1.x[0] + c1.x[1]) {
//     return 0;
//   } else if (c1.y[0] > c2.y[0] + c2.y[1] || c2.y[0] > c1.y[0] + c1.y[1]) {
//     return 0;
//   } else {
//     const xint = Math.min(c1.x[0]+c1.x[1], c2.x[0]+c2.x[1]) - Math.max(c1.x[0], c2.x[0]);
//     const yint = Math.min(c1.y[0]+c1.y[1], c2.y[0]+c2.y[1]) - Math.max(c1.y[0], c2.y[0]);
//     return xint * yint;
//   }
// }

function compute_label_pos(
  ft : d3.HierarchyNode<flat_node>, 
  label_height : number,
  y_scale : d3.ScaleLinear<number, number, never>,
  scale: number,
  layout_format : 'normal' | 'long') {

  const label_gap = y_scale.invert(consts.label_gap);
  for(const desc of ft) {
    const node = desc.data;
    const bottom_y = [node.x_pos + y_scale.invert(scale * consts.node_size / 2) + label_gap, label_height]
    if(layout_format == "long") {
      node.label_y = bottom_y[0];
    } else {
      const top_y = [bottom_y[0] - y_scale.invert(scale * consts.node_size) - bottom_y[1] - 2*label_gap, bottom_y[1]]
      node.label_y = node.depth % 2 == 0 ? top_y[0] : bottom_y[0];
    }
  }
}

// function compute_label_pos(
//   ft : d3.HierarchyNode<flat_node>, 
//   label_height : number,
//   x_scale : d3.ScaleLinear<number, number, never>,
//   y_scale : d3.ScaleLinear<number, number, never>) {

//   const label_gap = y_scale.invert(2);
//   const label_coords : coord[] = [];
  
//   for(const desc of ft) {
//     const node = desc.data;
//     if(node.x_pos == null || node.shortname == null) {
//       throw new Error("Cannot compute label positions before x coordinate and short name determined.");
//     } else {
//       const label_width = x_scale.invert(15) + compute_width(node.param_names, x_scale);
//       const start_coord : coord = {
//         x: [node.ered, label_width],
//         y: [node.x_pos + y_scale.invert(4.5) + label_gap, label_height]
//       };
//       const check_coords = label_coords
//                              .filter((coord) => intersect(coord.x, start_coord.x, 0.0))
//                              .sort((c1, c2) => c2.y[0] - c1.y[0])

//       let viable_y : number | undefined = undefined;
//       if(check_coords.length == 0) {
//         viable_y = start_coord.y[0];
//       } else {
//         let any_intersect = false;
//         for(const coord of check_coords) {
//           if(intersect(start_coord.y, coord.y, 0.0)) {
//             any_intersect = true;
//           }
//         }
//         if(!any_intersect) {
//           viable_y = start_coord.y[0];
//         }
//       }

//       if(viable_y == null) {

//         const bottom_coord = start_coord;
//         const top_coord : coord = {
//           x: [start_coord.x[0], start_coord.x[1]],
//           y: [start_coord.y[0] - y_scale.invert(9) - start_coord.y[1] - 2*label_gap, start_coord.y[1]]
//         };

//         let top_int = 0;
//         let bottom_int = 0;

//         for(let ii = 0; ii < check_coords.length; ++ii) {
//           const cur_coord = check_coords[ii];
//           top_int += intersect_area(top_coord, cur_coord);
//           bottom_int += intersect_area(bottom_coord, cur_coord);
//         }

//         if(top_int < bottom_int) {
//           viable_y = top_coord.y[0];
//         } else {
//           viable_y = bottom_coord.y[0];
//         }
//       }

//       // If default label position results in intersection, adjust position
//       // if(viable_y == null) {
//       //   let viable_dist = Infinity;
//       //   // For each label that overlaps start_coord in the x dimension (i.e. each label in the same "column")
//       //   let move_str = "";
//       //   for(let ii = 0; ii < check_coords.length; ++ii) {
//       //     const cur_coord = check_coords[ii];

//       //     // If there is room between labels to place current label below label ii
//       //     if(ii + 1 < check_coords.length && check_coords[ii][0] - check_coords[ii+1][0] > 2 * start_coord.y[1]) {
//       //       // Determine distance between this below placement and original label coordinates
//       //       const pos_dist_b = Math.abs((cur_coord[0] - cur_coord[1]) - start_coord.y[0]);
//       //       if(pos_dist_b < viable_dist) {
//       //         move_str = "below " + ii;
//       //         viable_y = cur_coord[0] - cur_coord[1];
//       //         viable_dist = pos_dist_b;
//       //       }
//       //     }
    
//       //     // If there is room between labels to place current label above label ii
//       //     if(ii > 0 && check_coords[ii - 1][0] - check_coords[ii][0] > 2 * start_coord.y[1]) {
//       //       // Determine distance between this above placement and original label coordinates
//       //       const pos_dist_t = Math.abs(cur_coord[0] + cur_coord[1] - start_coord.y[0]);
//       //       if(pos_dist_t < viable_dist) {
//       //         move_str = "above " + ii;
//       //         viable_y = cur_coord[0] + cur_coord[1];
//       //         viable_dist = pos_dist_t;
//       //       }
//       //     }
//       //   }
//       //   console.warn(`Moved ${move_str}.`)
//       // }

//       // if(viable_y >= node.x_pos) {
//       //   viable_y += 0.008;
//       // } else {
//       //   viable_y -= 0.016;
//       // }

//       // viable_y += 0.003;

//       //console.log("Setting label_y to ", viable_y);
//       node.label_y = viable_y;

//       const new_coord = start_coord;
//       new_coord.y = [viable_y, start_coord.y[1]];
//       label_coords.push(new_coord);
//     }
//   }
// }