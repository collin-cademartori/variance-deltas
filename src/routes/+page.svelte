<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket";
  import * as d3 from "d3";
  import { annotate_tree, type flat_node, type flat_branch } from "$lib/tree";
  import { get_tree, reset_tree, divide_branch, extrude_branch } from "$lib/tree_methods";
  import { draw_tree, reset_styles } from "$lib/draw_tree";
  import { setup_context } from "$lib/compute_width";

  import ExtrusionDialog from "$lib/components/ExtrusionDialog.svelte";

  const height = 500;
  const width = 1200;

  const x = d3.scaleLinear([0, 1], [0, width]);
  const y = d3.scaleLinear([0, 1], [0, 0.95 * height]);
  const l_height = 0.045;

  //const user_states = ['base', 'extruding'] as const;
  //type user_state_t = typeof user_states[number];
  type user_state_t = 'base' | 'extruding';
  let user_state : user_state_t = $state('base');

  let selected_node : flat_node | undefined = $state(undefined);

  $effect(() => {
    if(user_state === 'base') {
      reset_styles();
      selected_node = undefined;
    }
  });

  onMount(() => {
    setup_context(new OffscreenCanvas(1000, 1000));

    const svg = d3.select("#tree");
    let xaxis = d3.axisBottom(x);
    xaxis(svg.select("#x_axis"));

    ws.handle_message((data) => {
      const tree = annotate_tree(data, l_height, height, x);
      draw_tree(
        tree, x, y, l_height,
        (d : flat_node) => {
          if(selected_node?.name == d.name) {
            selected_node = undefined;
            return({
              // "border-color": "blue"
              "background-color" : "black",
              "color" : "white",
              "opacity" : 1
            });
          }
          if(user_state === 'extruding') {
            // extrude_branch({  
            //   node_name: parseInt(d.name),
            //   params_kept: d.params.slice(Math.ceil(d.params.length / 2))
            // });
            selected_node = d;
            return({
              // "border-color": "blue"
              "background-color" : "black",
              "color" : "white",
              "opacity" : 1
            });
          } else {
            return({});
          }
        },
        (d : flat_branch) => {
          // divide_branch({
          //   node_name: parseInt(d.child.name),
          //   params_kept: d.parent.params[9]
          // });
          return({
            "stroke": "blue"
          });
        },
        document.styleSheets[0]
      );
      console.log("Tree is now:")
      console.log(tree);
    });
    get_tree([]);

    document.addEventListener("keydown", (ev) => {
      if(ev.code == "KeyR") reset_tree([]);
    });

  });
</script>

<div id="v_container">

  <div id="menu_bar">
    <button 
      class:extruding={user_state === 'extruding'}
      onclick={() => user_state === 'extruding' ? user_state = 'base' : user_state = 'extruding'}
    >
      Extrude
    </button>
  </div>

  <div id="instruction_bar">
    {#if user_state == 'extruding'}
      <span>Select the node you would like to extrude.</span>
    {/if}
  </div>

  <div id="main_view">
      <div id="tree_container">
      <svg id="tree" height={height} width={width + 100}>
        <g id="tree_g" transform="translate(10 0)"></g>
        <g id="x_axis" transform={`translate(10 ${0.96 * height})`}></g>
      </svg>
    </div>

    <div id="control_container">
      {#if user_state === 'extruding' && selected_node != null}
        <ExtrusionDialog selected={selected_node} />
      {/if}
    </div>
  </div>

</div>

<style>
  #v_container { 
    display: flex;
    flex-direction: column;
  }

  #main_view {
    display: flex;
    flex-direction: row;
  }

  #menu_bar {
    display: flex;
    flex-direction: row;
  }

  :global(button) {
    appearance: none;
    border: 0.1rem solid black;
    background-color: white;
    border-radius: 0.1rem;
    padding: 0.2rem 0.25rem 0.2rem 0.25rem;  
    margin: 0.05rem 0 0.05rem 0;
  }

  :global(button:hover) {
    background-color: rgb(238, 238, 238);
  }

  :global(button:active) {
    background-color: rgb(211, 211, 211);
  }

  #instruction_bar {
    height: 1.5rem;
    font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    padding: 0.2rem;
  }

  .extruding {
    background-color: black;
    color: white;
  }

  #tree_container {
    display: flex;
    flex-direction: row;
  }
</style>