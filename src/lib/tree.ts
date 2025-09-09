import * as d3 from "d3";
import { short_name } from "./short_names.ts";
import { compute_width } from "./compute_width.ts";
import { D } from "../../.svelte-kit/output/server/chunks/index.js";

export type flat_node = {
  name: string,
  parent: string,
  ered: number,
  params: string[],
  sortname?: string,
  shortname?: string,
  x_pos?: number,
  label_y?: number
};
export type flat_branch = {
  child: flat_node,
  parent: flat_node
}
export type flat_tree = Array<flat_node>;
export type tree_node = d3.HierarchyNode<flat_node>;

type coord = {
  x: [number, number],
  y: [number, number]
}

export function annotate_tree(ft : flat_tree,
  label_height : number, max_height : number,
  x_scale : d3.ScaleLinear<number, number, never>
) {
  console.log("Setting short names.")
  for(const node of ft) {
    node.shortname = short_name(node.params);
  }
  console.log("Setting xs.")
  const tree_with_xs = compute_xs(ft);
  compute_label_pos(tree_with_xs, label_height, max_height, x_scale);
  return(tree_with_xs);
}

function compute_xs(ft : flat_tree) {
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
  const num_leaves = test_tree.leaves().length;
  for(const [lindex, leaf] of test_tree.leaves().sort(leaf_sort).entries()) {
    leaf.data.x_pos = 0.05 + 0.9 * ((lindex + 0.1) / ((num_leaves - 1) + 0.2));
  } 

  test_tree.eachAfter((d) => {
    if(d.data.x_pos == null) {
      const children = d.children;
      if(children == null) {
        throw new Error("Found leaf node without x_pos!");
      } else {
        const x_tot = children.reduce((p, n) => 
          p + (n.data.x_pos as number),
        0);
        d.data.x_pos = x_tot / children.length;
      }
    }
  });

  const new_ft : flat_tree = [...test_tree].map((n) => n.data);

  return(new_ft)
}

function intersect(x1 : [number, number], x2: [number, number]) {
  if((x2[0] + x2[1]) < x1[0] || x2[0] > (x1[0] + x1[1])) {
    return(false);
  } else {
    return(true);
  }
}

function compute_label_pos(
  ft : flat_tree, 
  label_height : number, max_height : number,
  x_scale : d3.ScaleLinear<number, number, never>) {
  const label_coords : coord[] = [];
  for(const node of ft) {
    if(node.x_pos == null || node.shortname == null) {
      throw new Error("Cannot compute label positions before x coordinate and short name determined.");
    } else {
      const label_width = x_scale.invert(15) + compute_width(node.shortname, x_scale);
      console.log("Label is ", node.shortname);
      console.log("X is ", node.ered);
      console.log("Width is ", label_width);
      const start_coord : coord = {
        x: [node.ered, label_width],
        y: [node.x_pos, label_height]
      };
      const check_coords = label_coords
                             .filter((coord) => intersect(coord.x, start_coord.x))
                             .sort((c1, c2) => c2.y[0] - c1.y[0])
                             .map((coord) => coord.y);

      let viable_y : number | undefined = undefined;
      if(check_coords.length == 0) {
        console.info("No collisions possible, proceeding.")
        viable_y = node.x_pos;
      } else {
        let any_intersect = false;
        for(const coord_y of check_coords) {
          if(intersect(start_coord.y, coord_y)) {
            any_intersect = true;
          }
        }
        if(any_intersect) {
          console.warn(`Label collision with ${label_coords.length} labels when placing ${node.shortname}! Adjusting y.`);
          // check_coords.push([max_height, label_height]); 
        } else {
          console.info("No collisions, proceeding.");
          viable_y = node.x_pos;
        }
      }

      if(viable_y == null) {
        console.log(check_coords);
        let viable_dist = Infinity;
        // let top_y = 0;
        for(let ii = 0; ii < check_coords.length; ++ii) {
          const cur_coord = check_coords[ii];

          if(ii + 1 < check_coords.length && check_coords[ii][0] - check_coords[ii+1][0] > 2 * start_coord.y[1]) {
            const pos_dist_b = Math.abs((cur_coord[0] + cur_coord[1]) - start_coord.y[0]);
            if(pos_dist_b < viable_dist) {
              viable_y = cur_coord[0] + cur_coord[1];
              viable_dist = pos_dist_b;
            }
          }
    
          if(ii > 0 && check_coords[ii - 1][0] - check_coords[ii][0] > 2 * start_coord.y[1]) {
            const pos_dist_t = Math.abs(cur_coord[0] - cur_coord[1] - start_coord.y[0]);
            if(pos_dist_t < viable_dist) {
              viable_y = cur_coord[0] - cur_coord[1];
              viable_dist = pos_dist_t;
            }
          }

          // top_y = cur_coord[0] + cur_coord[1];

          // if(cur_coord[0] - top_y >= start_coord.y[1]) {

          //   let pos_dist = Math.abs(cur_coord[0] - start_coord.y[0]);
          //   console.log(`Found label below starting position ${pos_dist}`);
          //   if(pos_dist < viable_dist) {
          //     viable_y = cur_coord[0] - start_coord.y[1];
          //     viable_dist = pos_dist;
          //   }
    
          //   pos_dist = Math.abs(top_y - start_coord.y[0]);
          //   if(pos_dist < viable_dist) {
          //     viable_y = top_y;
          //     viable_dist = pos_dist;
          //   }
          // }
          // top_y = cur_coord[0] + cur_coord[1];
        }
      }

      if(viable_y >= node.x_pos) {
        viable_y += 0.008;
      } else {
        viable_y -= 0.016;
      }

      console.log("Setting label_y to ", viable_y);
      node.label_y = viable_y;

      const new_coord = start_coord;
      new_coord.y = [viable_y, start_coord.y[1]];
      label_coords.push(new_coord);
    }
  }
}