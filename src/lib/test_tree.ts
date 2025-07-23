import * as d3 from "d3";

export type flat_node = {
  name: string,
  parent: string,
  ered: number,
  sortname?: string,
  x_pos?: number
};
type flat_tree = Array<flat_node>;
export type tree_node = d3.HierarchyNode<flat_node>;

const test_tree_flat : flat_tree = [
  { name: "mu_b[39,49]", parent: "", ered: 0 },
  { name: "mu_b[38,1-50]", parent: "mu_b[39,49]", ered: 0.07 },
  { name: "mu_b[37,49],e_bias[49]", parent: "mu_b[38,1-50]", ered: 0.14 },
  { name: "mu_b[37,49]", parent: "mu_b[38,1-50]", ered: 0.12 },
  { name: "n_democrat_potential[2]", parent: "mu_b[37,49],e_bias[49]", ered: 0.9 },
  { name: "n_democrat_potential[1]", parent: "mu_b[37,49]", ered: 0.88 },
  { name: "n_democrat_potential[3]", parent: "mu_b[37,49]", ered: 1 }
];

export const flat_tree = compute_xs(test_tree_flat);

function compute_xs(ft : flat_tree) {
  const test_tree = (d3.stratify<flat_node>()
                    .id((n : flat_node) => n.name)
                    .parentId((n : flat_node) => n.parent))(ft);

  for(const node of test_tree) {
    node.data.sortname = node.ancestors()
      .map((n) => n.id)
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
    leaf.data.x_pos = (lindex + 0.1) / (num_leaves + 0.2);
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