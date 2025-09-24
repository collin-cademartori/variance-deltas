<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket";
  import * as d3 from "d3";
  import { type flat_node, type flat_branch, type flat_tree } from "$lib/tree";
  import { get_tree, reset_tree, divide_branch, extrude_branch, auto_divide, merge_nodes, auto_merge, define_group } from "$lib/tree_methods";
  import { setup_context } from "$lib/compute_width";
  import { node_selection, clear_selection, branch_selection, clear_branches, selector } from "$lib/selection.svelte";
  import { user_state, setup_tree } from "$lib/user_state.svelte";
  import { groups, handle_groups } from "$lib/groups";

  import SelectionDialog from "$lib/components/SelectionDialog.svelte";
  import MultiSelectionDialog from "$lib/components/MultiSelectionDialog.svelte";
  import GroupsDialog from "$lib/components/GroupsDialog.svelte";
  import ExportDialog from "$lib/components/ExportDialog.svelte";

  let show_export = $state(false);

  $effect(() => {
    console.log(`Show export is now ${show_export}`)
  })

  const height = 700;
  const width = 1150;

  const x = d3.scaleLinear([0, 1], [0, 0.95 * width]);
  const y = d3.scaleLinear([0, 1], [0, 0.95 * height]);
  const l_height = 36;

  let selected_node = $derived.by(() => user_state.tree?.find((node) => node.data.name === [...node_selection.main.values()][0])?.data);
  let selected_alt_node = $derived.by(() => user_state.tree?.find((node) => node.data.name === [...node_selection.alt.values()][0])?.data);
  let selected_branch = $derived.by(() => {
    const nodes = [...branch_selection.main][0];
    return({
      child: user_state.tree?.find((node) => node.data.name === nodes?.[0])?.data,
      parent: user_state.tree?.find((node) => node.data.name === nodes?.[1])?.data
    });
  });

  onMount(() => {
    setup_context(new OffscreenCanvas(1000, 1000));

    const svg = d3.select("#tree");
    let xaxis = d3.axisBottom(x).offset(10).tickPadding(15).tickSize(0);
    xaxis(svg.select("#x_axis"));

    setup_tree(
      x, y, l_height,
      (d : flat_node) => {
        if(user_state.state === 'extruding') {
          if(node_selection.main.has(d.name)) {
            node_selection.main.clear();
            return({ target: [], channel: 'main' })
          } else {
            node_selection.main.clear();
            node_selection.main.add(d.name);
            return({ target: [d.name], channel: 'main' })
          }
        } else if (user_state.state === 'merging') {
          if(node_selection.main.has(d.name)) {
            clear_selection();
            return(null);
          } else if (node_selection.alt.has(d.name)) {
            node_selection.alt.clear();
            return({ target: [], channel: 'alt' });
          } else if (node_selection.main.size > 0) {
            node_selection.alt.clear();
            node_selection.alt.add(d.name);
            return({ target: [d.name], channel: 'alt' });
          } else {
            node_selection.main.clear();
            node_selection.main.add(d.name);
            return({ target: [d.name], channel: 'main' });
          }
        } else if (user_state.state === 'add-group') {
          const sel_node = user_state.tree?.find((node) => node.data.name === d.name);
          let sel_names : string[] = [];
          if(selector.type === "anc") {
            if(sel_node?.data.name && node_selection.main.has(sel_node?.data.name)) {
              sel_names = sel_node?.descendants().map((desc) => desc.data.name);
              sel_names.forEach((node_name) => node_selection.main.delete(node_name));
              return({
                target: [...node_selection.main], channel: 'main'
              });
            } else {
              sel_names = sel_node?.ancestors().map((anc) => anc.data.name) ?? [];
              sel_names.forEach((name) => node_selection.main.add(name));
              return({
                target: [...node_selection.main], channel: 'main'
              });
            }
          } else if(selector.type === "desc") {
            if(sel_node?.data.name && node_selection.del.has(sel_node?.data.name)) {
              sel_names = sel_node?.ancestors().map((anc) => anc.data.name);
              sel_names.forEach((node_name) => node_selection.del.delete(node_name));
              return({
                target: [...node_selection.del], channel: 'del'
              });
            } else {
              sel_names = sel_node?.descendants().map((desc) => desc.data.name) ?? [];
              sel_names.forEach((name) => node_selection.del.add(name));
              return({
                target: [...node_selection.del], channel: 'del'
              });
            }
          } else {
            return(null);
          }
        } else {
          return(null);
        }
      },
      (d : flat_branch) => {
        if(user_state.state === 'dividing' || user_state.state === 'auto-dividing') {
          if(selected_branch.parent?.name == d.parent.name && selected_branch.child?.name == d.child.name) {
            clear_branches();
          } else {
            branch_selection.main.clear();
            branch_selection.main.add([d.child.name, d.parent.name]);
          }
          return({
            "stroke-dasharray" : "5px, 5px"
          });
        } else {
          return({});
        }
      }
    );

    ws.handle_message((tree_data, globals_data, groups_data) => {
      const tree = (d3.stratify<flat_node>()
                    .id((n : flat_node) => n.name.toString())
                    .parentId((n : flat_node) => n.parent.toString()))(tree_data);
      handle_groups(groups_data);
      globals_data.forEach((global) => user_state.globals.push(global));
      user_state.tree = tree;
    });
    ws.handle_groups(handle_groups);
    get_tree([]);

    document.addEventListener("keydown", (ev) => {
      if(ev.code == "KeyR") reset_tree([]);
    });

  });
</script>

<div id="v_container">

  <ExportDialog bind:show_dialog={show_export} plot_width={width} {y} />

  <div id="main_view">
      <div id="tree_container">
      <svg id="tree" height={height} width={width + 100}>
        <linearGradient id="grad" x1="0" x2="0" y1="0" y2="0.04">
          <stop class="stop1" offset="0%" stop-color="white" stop-opacity="0" />
          <stop class="stop2" offset="30%" stop-color="white"/>
          <stop class="stop3" offset="100%" stop-color="white"/>
        </linearGradient>
        
        <g id="tree_outer" transform="translate(10 0)">
          <rect width="100%" height="100%" fill="white"></rect>
          <g id="tree_g"></g>
          <g id="tree_g_selection">
          </g>
        </g>
        
        <g id="x_axis" transform={`translate(10 ${0.96 * height})`}>
          <rect width="100%" height="100%" fill="url(#grad)"></rect>
          <g id="x_axis"></g>
        </g>
      </svg>
    </div>

    <div id="control_container">
      <div id="session_bar" class="button_bar">
        <button onclick={() => {
          show_export = true;
        }}>
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
      </div>

      <div id="instruction_bar">
        {#if user_state.state === 'extruding'}
          <span>Select the node you would like to extrude.</span>
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
            node_selection.main.size > 0 ? 
            [...node_selection.main].map((name) => user_state.tree?.find((node) => node.data.name === name)?.data) :
            [...node_selection.del].map((name) => user_state.tree?.find((node) => node.data.name === name)?.data)
          } 
          input_text={"Group Name"}
          button_text={"Create Group"} 
          button_action={(name, node_names) => {
            // console.log(node_names);  
            // make_group(name, node_names);
            define_group({
              group_name: name,
              node_names: node_names
            });
            node_selection.main.clear();
            node_selection.del.clear();
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
      {/if}
    </div>
  </div>

</div>

<style>
  :global(.main_label_selected) {    
    border-color: rgb(29, 29, 212) !important;
    background-color: rgb(224, 235, 255) !important;
    opacity: 1 !important;
  }

  :global(.alt_label_selected) {
    border-color: rgb(145, 10, 138) !important;
    background-color: rgb(241, 232, 240) !important;
    opacity: 1 !important;
  }

  :global(.del_label_selected) {
    border-color: rgb(212, 142, 29) !important;
    background-color: rgb(255, 236, 224) !important;
    opacity: 1 !important;
  }

  :global(.main_rect_selected) {    
    fill: rgb(64, 64, 234) !important;
    opacity: 1 !important;
  }

  :global(.alt_rect_selected) {    
    fill: rgb(145, 10, 138) !important;
    opacity: 1 !important;
  }

  :global(.del_rect_selected) {    
    fill: rgb(212, 142, 29) !important;
    opacity: 1 !important;
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

  #tree_container {
    display: flex;
    flex-direction: row;
  }

  #control_container {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    padding-top: 2rem;
    padding-left: 1rem;
    width: 100%;
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