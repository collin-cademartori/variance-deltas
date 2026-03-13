// Types that define flat tree structure

export type flat_node = {
  name: string,
  parent: string,
  ered: number,
  params: string[],
  lwidth? : number,
  vspace?: number,
  depth? : number,
  sortname?: string,
  shortname?: string,
  param_names?: string[],
  x_pos?: number,
  label_y?: number
};
export type flat_branch = {
  child: flat_node,
  parent: flat_node
}
export type flat_tree = Array<flat_node>;
export type tree_node = d3.HierarchyNode<flat_node>;