<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket";
  import * as d3 from "d3";
  import { annotate_tree } from "$lib/tree";
  import { get_tree, reset_tree } from "$lib/tree_methods";
  import { draw_tree } from "$lib/draw_tree";
  import { setup_context } from "$lib/compute_width";

  const height = 500;
  const width = 1200;

  const x = d3.scaleLinear([0, 1], [0, width]);
  const y = d3.scaleLinear([0, 1], [0, 0.95 * height]);
  const l_height = 0.045;

  const user_state = ['base', 'extruding'] as const;

  onMount(() => {
    setup_context(new OffscreenCanvas(1000, 1000));

    const svg = d3.select("#tree");
    let xaxis = d3.axisBottom(x);
    xaxis(svg.select("#x_axis"));

    ws.handle_message((data) => {
      const tree = annotate_tree(data, l_height, height, x);
      draw_tree(tree, x, y, l_height);
      console.log("Tree is now:")
      console.log(tree);
    });

    draw_tree([], x, y, l_height);
    get_tree([]);

    document.addEventListener("keydown", (ev) => {
      if(ev.code == "KeyR") reset_tree([]);
    });

  });
</script>

<div id="tree_container">
  <svg id="tree" height={height} width={width + 100}>
    <g id="tree_g" transform="translate(10 0)"></g>
    <g id="x_axis" transform={`translate(10 ${0.96 * height})`}></g>
  </svg>
</div>

<style>
  #tree_container {
    display: flex;
    flex-direction: row;
  }
</style>