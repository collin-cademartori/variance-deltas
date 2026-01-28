<svelte:head>
   <link id="katex_style" rel="stylesheet" href="/katex.css">
</svelte:head>

<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket.svelte";
  import { connection } from "$lib/websocket.svelte";
  import * as d3 from "d3";
  import type { flat_node, flat_branch } from "$lib/state/types";
  import { get_tree, reset_tree, divide_branch, extrude_branch, auto_divide, merge_nodes, auto_merge, delete_node } from "$lib/tree_methods";
  // import { setup_context } from "$lib/state/compute_width";
  import { selection } from "$lib/state/selection.svelte";
  import { user_state, setup_tree, update_names } from "$lib/state/user_state.svelte";
  import { groups, make_group, prune_groups } from "$lib/state/groups";

  import SelectionDialog from "$lib/components/SelectionDialog.svelte";
  import MultiSelectionDialog from "$lib/components/MultiSelectionDialog.svelte";
  import GroupsDialog from "$lib/components/GroupsDialog.svelte";
  import ExportDialog from "$lib/components/ExportDialog.svelte";
  import SettingsDialog from "$lib/components/SettingsDialog.svelte";
  import ConnectionOverlay from "$lib/components/ConnectionOverlay.svelte";

  let show_export = $state(false);

  const width = 1150;

  const x = d3.scaleLinear([0, 1], [0, 0.85 * width]);
  const y = d3.scaleLinear([0, 1], [0, 1000]);
  const l_height = 50;

  let selected_node = $derived.by(() => user_state.tree?.find((node) => node?.data.name === selection.nodes("main")?.[0])?.data);
  let selected_alt_node = $derived.by(() => user_state.tree?.find((node) => node?.data.name === selection.nodes("alt")?.[0])?.data);
  let selected_branch = $derived.by(() => {
    const node1_name = selection.nodes("main")[0];
    const node2_name = selection.nodes("main")[1];
    const node1 = user_state.tree?.find((node) => node?.data.name === node1_name);
    if(node1?.data.parent == node2_name) {
      return({
        child: node1?.data,
        parent: user_state.tree?.find((node) => node?.data.name === node2_name)?.data
      });
    } else {
      return({
        child: undefined,
        parent: undefined
      });
    }
  });

  $effect(() => {
    console.log(`SVG height is now ${user_state.svg_height}`)
  })

  onMount(() => {
    // setup_context(new OffscreenCanvas(1000, 1000));

    // computed_height = 0.9 * window.innerHeight;
    // computed_height = tree_svg?.getBBox().height ?? 0;

    const axis_svg = d3.select("#axis");
    let xaxis = d3.axisBottom(x).offset(2).tickPadding(7).tickSize(4);
    xaxis(axis_svg.select("#x_axis"));
    axis_svg.select("#x_axis").attr("font-size", (l_height * (12 / 36)) + "px");

    setup_tree({x: x, y: y}, l_height);

    ws.handle_message((tree_data, globals_data, global_limit, _groups_data) => {
      try {
        const pnames = new Set([...tree_data].map((node) => node.params.map((n) => n.split("[")[0])).flat());
        console.log("Step 1")
        update_names(pnames);
        console.log("Step 2")
        const tree = (d3.stratify<flat_node>()
                      .id((n : flat_node) => n.name.toString())
                      .parentId((n : flat_node) => n.parent.toString()))(tree_data);
        console.log("Step 3")
        // Prune groups that reference deleted nodes
        const valid_node_names = new Set(tree_data.map((node: flat_node) => node.name));
        prune_groups(valid_node_names);
        console.log("Step 4")
        globals_data.forEach((global) => user_state.globals.push(global));
        console.log("Step 5")
        user_state.global_limit = global_limit;
        user_state.tree = tree;
        console.log("Step 6")
      } catch (err) {
        console.error("Error while handling tree message:");
        console.error(err);
      }

    });
    get_tree([]);

    // document.addEventListener("keydown", (ev) => {
    //   if(ev.code == "KeyR") reset_tree([]);
    // });

  });
</script>

<div id="v_container">

  <ExportDialog bind:show_dialog={show_export} plot_width={width} {y} />

  <div id="main_view">
    <div id="vis_container">
      <div id="tree_container">
        <svg id="tree" height={user_state.svg_height} width={width + 100}>
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

          <g id="top_bar" transform="translate(20 0)">
            <rect fill="white" height="40" width="100%" transform="translate(-20 0)"></rect> 
          </g>
        </svg>
      </div>
      <div id="axis_container">
        <svg id="axis" height={65} width={width + 100}>
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
    </div>

    <div id="control_container" class:frozen={connection.frozen}>
      <ConnectionOverlay />
      <div id="control_content">
      <div id="session_bar" class="button_bar">
        <button 
          onclick={() => {
            show_export = true;
          }}
        >
          Export Image
        </button>
        <div class="button_group">
          <button
            class:menu_enabled={user_state.state === 'groups'}
            onclick={() => user_state.state === 'groups' ? user_state.state = 'base' : user_state.state = 'groups'}
          >
            Groups
          </button>
          <button
            class:menu_enabled={user_state.state === 'add-group'}
            onclick={() => user_state.state === 'add-group' ? user_state.state = 'base' : user_state.state = 'add-group'}
          >
            Add Group
          </button>
        </div>

        <button
          class:menu_enabled={user_state.state === 'settings'}
          onclick={() => user_state.state === 'settings' ? user_state.state = 'base' : user_state.state = 'settings'}
        >
          Settings
        </button>
      </div>

      <div id="menu_bar" class="button_bar">
        <button 
          class:menu_enabled={user_state.state === 'extruding'}
          onclick={() => user_state.state === 'extruding' ? user_state.state = 'base' : user_state.state = 'extruding'}
        >
          Extrude
        </button>
        <div class="button_group">
          <button 
            class:menu_enabled={user_state.state === 'dividing'}
            onclick={() => user_state.state === 'dividing' ? user_state.state = 'base' : user_state.state = 'dividing'}
          >
            Divide
          </button>
          <button 
            class:menu_enabled={user_state.state === 'auto-dividing'}
            onclick={() => user_state.state === 'auto-dividing' ? user_state.state = 'base' : user_state.state = 'auto-dividing'}
          >
            Auto
          </button>
        </div>
        <div class="button_group">
          <button 
            class:menu_enabled={user_state.state === 'merging'}
            onclick={() => user_state.state === 'merging' ? user_state.state = 'base' : user_state.state = 'merging'}
          >
            Merge
          </button>
          <button 
            class:menu_enabled={user_state.state === 'auto-merging'}
            onclick={() => user_state.state === 'auto-merging' ? user_state.state = 'base' : user_state.state = 'auto-merging'}
          >
            Auto
          </button>
        </div>
        <button 
          class:menu_enabled={user_state.state === 'deleting'}
          onclick={() => user_state.state === 'deleting' ? user_state.state = 'base' : user_state.state = 'deleting'}
        >
          Delete
        </button>
      </div>

      <div id="instruction_bar">
        {#if user_state.state === 'extruding'}
          <span>Select the node you would like to extrude.</span>
        {:else if user_state.state === 'deleting'}
          <span>Select the node you would like to delete.</span>
        {:else if user_state.state === 'dividing'}
          <span>Select the branch you would like to divide.</span>
        {:else if user_state.state === 'auto-dividing'}
          <span>Select branch you would like to automatically divide.</span>
        {:else if user_state.state === 'merging'}
          <span>Select the nodes you would like merge.</span>
        {:else if user_state.state === 'auto-merging'}
          <span>Press the button to begin auto-merge.</span>
        {/if}
      </div>

      {#if user_state.state === 'add-group'}
        <MultiSelectionDialog 
          selected={
            selection.size("main") > 0 ? 
            selection.nodes("main").map((name) => user_state.tree?.find((node) => node.data.name === name)?.data) :
            selection.nodes("del").map((name) => user_state.tree?.find((node) => node.data.name === name)?.data)
          } 
          input_text={"Group Name"}
          button_text={"Create Group"} 
          button_action={(name, node_names) => {
            make_group(name, node_names);
            selection.clear();
            user_state.state = 'groups';
          }} 
        />
      {:else if user_state.state === 'groups'}
          {#if groups.size > 0}
            <GroupsDialog />
          {:else}
            <span class="placeholder">No groups defined.</span>
          {/if}
      {:else if user_state.state === 'extruding' && selected_node != undefined}
        <SelectionDialog 
          selected={selected_node} 
          button_text={"Extrude"} 
          button_action={
            (params) => extrude_branch({ 
              node_name : parseInt((selected_node as flat_node).name),
              params_kept: params 
            })
          } 
        />
      {:else if user_state.state === 'deleting' && selected_node != undefined}
        <SelectionDialog 
          selected={null} 
          button_text={"Delete"} 
          button_action={
            (params) => delete_node({ 
              node_name : parseInt((selected_node as flat_node).name),
            })
          } 
        />
      {:else if user_state.state === 'dividing' && selected_branch.parent != undefined}
        <SelectionDialog 
          selected={selected_branch.parent} 
          button_text={"Divide"}
          button_action={
            (params) => divide_branch({ 
              node_name : parseInt((selected_branch as flat_branch).child.name),
              params_kept: params 
            })
          }
        />
      {:else if user_state.state === 'auto-dividing' && selected_branch != undefined}
        <SelectionDialog
          selected={null}
          button_text={"Auto Divide"}
          button_action={
            () => auto_divide({ 
              node_name : parseInt((selected_branch as flat_branch).child.name),
            })
          }
        />
      {:else if user_state.state === 'merging' && selected_node != undefined && selected_alt_node != undefined}
        <SelectionDialog
          selected={null}
          button_text={"Merge"}
          button_action={
            () => merge_nodes({ 
              node_name : parseInt((selected_node as flat_node).name),
              alt_node_name : parseInt((selected_alt_node as flat_node).name)
            })
          }
        />
      {:else if user_state.state === 'auto-merging'}
        <SelectionDialog
          selected={null}
          button_text={"Auto Merge"}
          button_action={
            () => auto_merge({})
          }
        />
      {:else if user_state.state === 'settings'}
        <SettingsDialog />
      {/if}
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
    border: 0.1rem solid black;
    background-color: white;
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
  }

  #instruction_bar {
    height: 1.5rem;
    font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    padding: 0.2rem;
    display: flex;
    flex-direction: row;
  }

  :global(button.menu_enabled) {
    color: rgb(0, 0, 136);
    border-color: rgb(29, 29, 212);
    background-color: rgb(224, 235, 255);
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
    padding-left: 2rem;
    padding-right: 2rem;
    padding-top: 0.5rem;
    padding-bottom: 2rem;
  }

  #main_view {
    display: flex;
    flex-direction: row;
    height: 95dvh;
    overflow-y: hidden;
    overflow-x: hidden;
  }

  #vis_container {
    display: flex;
    flex-direction: column;
    min-width: 0;
    overflow-x: auto;
  }

  #tree_container {
    display: flex;
    flex-direction: row;
    overflow-y: auto;
    overflow-x: clip;
    flex-shrink: 0;
    width: fit-content;
  }

  #tree {
    flex-shrink: 0;
  }

  #axis_container {
    overflow-y: hidden;
    overflow-x: clip;
    width: fit-content;
  }

  #control_container {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    padding-top: 2rem;
    padding-left: 1rem;
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
    gap: 0.5rem;
  }

  .placeholder {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-style: italic;
    font-size: 0.8rem;
    color: rgb(110, 110, 110);
  }

  :global(body) {
    margin: 0;
  }
</style>