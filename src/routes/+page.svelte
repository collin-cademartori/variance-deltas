<script lang="ts">
  import { onMount } from "svelte";
  import { ws } from "$lib/websocket";
  import * as d3 from "d3";
  import { flat_tree, type flat_node } from "$lib/test_tree";

  const height = 600;
  const width = 600;

  console.log(flat_tree);

  onMount(() => {
    let svg = d3.select("#tree");
    let tree_elem = d3.select("#tree_g");

    let y = d3.scaleLinear([1, 0], [0, 0.95 * height]);
    let yaxis = d3.axisLeft(y);
    yaxis(svg.select("#y_axis"));

    let x = d3.scaleLinear([0, 1], [0, 1 * width]);
    // let xaxis = d3.axisBottom(x);
    // xaxis(svg.select("#x_axis"));

    for(const node of flat_tree) {
      const pnode = flat_tree.find((n) => n.name === node.parent);
      if(pnode != null) {
        const link = d3.line<flat_node>(
          (d) => 50 + x(d.x_pos ?? 0),
          (d) => y(d.ered)
        ).curve(d3.curveStepAfter);
        tree_elem.append("path").attr("d", link([pnode, node]))
          .attr("stroke", "black")
          .attr("stroke-width", 1.5)
          .attr("fill", "none");
      }
    }

    tree_elem.selectAll(".tree_node").data(flat_tree)
      .enter().append("circle")
      .attr("class", "tree_node")
      .attr("r", 5)
      .attr("cx", (d : flat_node) => 50 + x(d.x_pos ?? 0))
      .attr("cy", (d: flat_node) => y(d.ered))
      .attr("fill", "black")
      .attr("stroke", "white")
      .attr("stroke-width", 4);
  });
</script>

<div id="tree_container">
  <svg id="tree" height={height} width={width}>
    <g id="y_axis" transform="translate(40 20)"></g>
    <g id="tree_g" transform="translate(40 20)"></g>
  </svg>
</div>