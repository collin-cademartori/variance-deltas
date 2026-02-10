<svelte:head>
   <link id="katex_style" rel="stylesheet" href="/katex.css">
</svelte:head>

<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket.svelte";
  import { connection } from "$lib/websocket.svelte";
  import * as d3 from "d3";
  import type { flat_node } from "$lib/state/types";
  import { get_tree } from "$lib/tree_methods";
  import { user_state, setup_tree, update_names } from "$lib/state/user_state.svelte";
  import { expand_groups,prune_groups } from "$lib/state/groups";

  import GroupsDialog from "$lib/components/GroupsDialog.svelte";
  import ExportDialog from "$lib/components/ExportDialog.svelte";
  import SettingsDialog from "$lib/components/SettingsDialog.svelte";
  import ConnectionOverlay from "$lib/components/ConnectionOverlay.svelte";
  import InlineEditor from "$lib/components/InlineEditor.svelte";
  import SaveButton from "$lib/components/SaveButton.svelte";
  import TreeEditor from "$lib/components/TreeEditor.svelte";

  let show_export = $state(false);

  const width = 1000;

  const x = d3.scaleLinear([0, 1], [0, width]);
  const y = d3.scaleLinear([0, 1], [0, 1000]);
  const l_height = 50;

  $effect(() => {
    console.log(`SVG height is now ${user_state.svg_height}`)
  })

  onMount(() => {
    const axis_svg = d3.select("#axis");
    let xaxis = d3.axisBottom(x).offset(2).tickPadding(7).tickSize(4);
    xaxis(axis_svg.select("#x_axis"));
    axis_svg.select("#x_axis").attr("font-size", (l_height * (12 / 36)) + "px");

    setup_tree({x: x, y: y}, l_height);

    ws.handle_message((tree_data, globals_data, global_limit, _groups_data, sid) => {
      console.log("Message received from backend.")
      try {
        // Set sid if present
        if(sid) {
          user_state.session_id = sid;
        }
        // Extract all parameter names from tree and update the local list of parameter names
        const pnames = new Set([...tree_data].map((node) => node.params.map((n) => n.split("[")[0])).flat());
        update_names(pnames);

        // Construct d3 hierarchy from array of tree nodes
        const tree = (d3.stratify<flat_node>()
                      .id((n : flat_node) => n.name.toString())
                      .parentId((n : flat_node) => n.parent.toString()))(tree_data);

        // Update groups to reflect deleted and added nodes
        const valid_node_names = new Set(tree_data.map((node: flat_node) => node.name));
        prune_groups(valid_node_names);
        expand_groups(tree);

        // Update globals
        globals_data.forEach((global) => user_state.globals.push(global));
        user_state.global_limit = global_limit;

        // Update tree in user state
        user_state.tree = tree;
      } catch (err) {
        console.error("Error while handling tree message:");
        console.error(err);
      }

    });

    get_tree([]);
  });
</script>

<div id="v_container">

  <ExportDialog bind:show_dialog={show_export} plot_width={Math.max(width, user_state.svg_width)} axis_width={width} {y} />

  <div id="session_top">
    <InlineEditor bind:title={user_state.title} />
    <button 
      onclick={() => {
        show_export = true;
      }}
    >
      Export Image
    </button>
    <SaveButton />
  </div>

  <div id="main_view">
    <div id="vis_container">
      <div id="tree_container">
        <svg id="tree" height={user_state.svg_height} width={40 + Math.max(width, user_state.svg_width)}>
          <rect width="100%" height="100%" fill="white"></rect>

          <g id="vert_line_container" transform="translate(20 0)">
          </g>
          
          <g id="tree_outer" transform="translate(20 20)">
            <g id="tree_layers">
              <g id="tree_g"></g>
              <g id="tree_g_del"></g>
              <g id="tree_g_alt"></g>
              <g id="tree_g_main"></g>
            </g>
          </g>

          <g id="label_layer_container" transform="translate(20 20)">
            <g id="label_layer"></g>
          </g>

          <!-- <g id="top_bar" transform="translate(20 0)">
            <rect fill="white" height="40" width="100%" transform="translate(-20 0)"></rect> 
          </g> -->
        </svg>
      </div>
      <div id="axis_container">
        <svg id="axis" height={65} width={width + 40}>
          <linearGradient id="grad" x1="0" x2="0" y1="0" y2="0.04">
            <stop class="stop1" offset="0%" stop-color="white" stop-opacity="0" />
            <stop class="stop2" offset="30%" stop-color="white"/>
            <stop class="stop3" offset="100%" stop-color="white"/>
          </linearGradient>

          <!-- <rect width="100%" height="100%" fill="white"></rect> -->
          
          <g id="x_axis_container" transform={`translate(20 0)`}>
            <rect width="100%" height="100" fill="url(#grad)"></rect>
            <g id="x_axis"></g>
          </g>

          <g id="highlight_container" transform={`translate(20 30)`}>
          </g>
        </svg>
      </div>
      <div id="measure_div" style:height="0" style:overflow="clip">
        <svg id="measure_container" height={user_state.svg_height} width={width + 100} style="visibility: hidden">
          <g id="label_measure_layer"></g>
        </svg>
      </div>
    </div>

    <div id="control_container" class:frozen={connection.frozen}>
      <ConnectionOverlay />
      <div id="control_content">
      <TreeEditor />
      <GroupsDialog />
      <SettingsDialog />
      </div>
    </div>
  </div>

</div>

<style>

  :global(.katex) {
    font-size: 0.9em !important;
  }

  :global(.main_label_selected) {    
    border-color: rgb(29, 29, 212) !important;
    background-color: rgb(224, 235, 255) !important;
    transition: all 50ms !important; 
  }

  :global(.main_label_selected > .params_list div) {    
    border-color: rgb(29, 29, 212) !important;
    transition: all 50ms !important; 
  }

  :global(.alt_label_selected) {
    border-color: rgb(145, 10, 138) !important;
    background-color: rgb(241, 232, 240) !important;
    transition: all 50ms !important; 
  }

  :global(.alt_label_selected > .params_list div) {    
    border-color: rgb(145, 10, 138) !important;
    transition: all 50ms !important; 
  }

  :global(.del_label_selected) {
    border-color: rgb(212, 142, 29) !important;
    background-color: rgb(255, 236, 224) !important;
    transition: all 50ms !important; 
  }

  :global(.del_label_selected > .params_list div) {    
    border-color: rgb(212, 142, 29) !important;
    transition: all 50ms !important; 
  }

  :global(.button_bar) {
    display: flex;
    flex-direction: row;
    gap: 0.5rem;
  }

  :global(button) {
    appearance: none;
    border: 0.1rem solid rgb(129, 129, 129);
    background-color: rgb(250, 250, 250);
    border-radius: 0.2rem;
    padding: 0.2rem 0.4rem 0.2rem 0.4rem;  
    margin: 0.05rem 0 0.05rem 0;
  }

  :global(button:hover) {
    background-color: rgb(238, 238, 238);
  }

  :global(button:active) {
    background-color: rgb(211, 211, 211);
  }

  :global(.button_group) {
    display: flex;
    flex-direction: row;
    height: fit-content;
  }

  :global(.button_group > button:first-child) {
    border-right: none;
    border-top-right-radius: 0;
    border-bottom-right-radius: 0;
    border-top-left-radius: 0.2rem;
    border-bottom-left-radius: 0.2rem;
  }

  :global(.button_group > button:last-child) {
    border-top-left-radius: 0;
    border-bottom-left-radius: 0;
    border-top-right-radius: 0.2rem;
    border-bottom-right-radius: 0.2rem;
    border-right: 0.1rem solid black;
  }

  :global(.button_group > button) {
    border-radius: 0;
    border-right: none;
    width: fit-content;
  }

  :global(button.menu_enabled) {
    color: rgb(0, 0, 136);
    border-color: rgb(29, 29, 212);
    background-color: rgb(224, 235, 255);
    box-shadow: 0 0 0.1rem rgb(187, 209, 250);
    /* border-color: rgb(65, 125, 255);
    background-color: rgb(65, 125, 255);
    color: white; */
  }

  :global(button.menu_enabled:active) {
    border-color: rgb(29, 29, 212);
    background-color: rgb(196, 215, 255);
    /* border-color: rgb(108, 154, 255);
    background-color: rgb(108, 154, 255);
    color: white; */
  }

  #v_container { 
    display: flex;
    flex-direction: column;
    /* padding-left: 2rem;
    padding-right: 2rem;
    padding-top: 0.5rem;
    padding-bottom: 2rem; */
    background-color: rgb(251, 251, 251);
    padding-left: 2rem;
    height: 100dvh;
  }

  #main_view {
    display: flex;
    flex-direction: row;
    overflow-y: hidden;
    overflow-x: hidden;
    height: 100%;
    padding-bottom: 2rem;
  }

  #session_top {
    display: flex;
    flex-direction: row;
    gap: 0.5rem;
    align-items: center;
    height: 3rem;
    padding: 0.5rem 0 0.5rem 0;
  }

  #vis_container {
    display: flex;
    flex-direction: column;
    min-width: 0;
    overflow-x: auto;
    overflow-y: clip;
    background-color: white;
    border-radius: 0.2rem;
    /* padding: 0 2rem 0 2rem; */
    border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    height: fit-content;
    max-height: 100%;
  }

  #tree_container {
    flex-basis: auto;
    flex-shrink: 2;
    flex-grow: 1;
    display: flex;
    flex-direction: row;
    overflow-y: auto;
    overflow-x: clip;
    width: fit-content;
    padding-top: 2rem;
    /* height: 100%; */
  }

  #vis_container > * {
    padding: 0 2rem 0 2rem;
  }

  /* #tree {
    flex-shrink: 0;
  } */

  #axis_container {
    flex-basis: 5rem;
    flex-shrink: 1;
    overflow-y: clip;
    overflow-x: clip;
    width: fit-content;
  }

  #control_container {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    /* padding-top: 1rem; */
    padding: 0 1.5rem 0 1.5rem;
    width: 22rem;
    flex-shrink: 0;
  }

  #control_container.frozen {
    pointer-events: none;
  }

  #control_container.frozen > #control_content {
    opacity: 0.6;
  }

  #control_content {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
  }

  :global(body) {
    margin: 0;
  }
</style>