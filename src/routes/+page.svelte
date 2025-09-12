<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket";
  import * as d3 from "d3";
  import { annotate_tree, type flat_node, type flat_branch } from "$lib/tree";
  import { get_tree, reset_tree, divide_branch, extrude_branch, auto_divide, merge_nodes, auto_merge } from "$lib/tree_methods";
  import { draw_tree, reset_styles } from "$lib/draw_tree";
  import { setup_context } from "$lib/compute_width";
  import { export_svg } from "$lib/export_svg";

  import SelectionDialog from "$lib/components/SelectionDialog.svelte";

  const height = 950; //950
  const width = 1150; //1150

  const x = d3.scaleLinear([0, 1], [0, width]);
  const y = d3.scaleLinear([0, 1], [0, 0.95 * height]);
  const l_height = 0.036;

  type user_state_t = 'base' | 'extruding' | 'dividing' | 'auto-dividing' | 'merging' | 'auto-merging';
  let user_state : user_state_t = $state('base');

  let selected_node : flat_node | undefined = $state(undefined);
  let selected_alt_node : flat_node | undefined = $state(undefined);
  let selected_branch : flat_branch | undefined = $state(undefined);

  $effect(() => {
    if(user_state === 'base') {
      selected_node = undefined;
      selected_branch = undefined;
      selected_alt_node = undefined;
    } else if (user_state === 'dividing' || user_state === 'auto-dividing') {
      selected_node = undefined;
      selected_alt_node = undefined;
    } else if (user_state === 'extruding') {
      selected_branch = undefined;
      selected_alt_node = undefined;
    } else if (user_state === 'merging') {
      selected_branch = undefined;
    } else if (user_state === 'auto-merging') {
      selected_branch = undefined;
      selected_branch = undefined;
      selected_alt_node = undefined;
    }
    reset_styles();
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
          if(user_state === 'extruding') {
            let action = "";
            if(selected_node?.name == d.name) {
              selected_node = undefined;
              action = "deselect";
            } else {
              selected_node = d;
              action = "select";
            }
            return({
              selections: 'main',
              action: action,
              styles: {
                
              }
            });
          } else if (user_state === 'merging') {
            let selections = "";
            let action = "";
            if(selected_node?.name == d.name) {
              selected_node = undefined;
              selected_alt_node = undefined;
              selections = "both";
              action = "deselect";
            } else if (selected_alt_node?.name == d.name) {
              selected_alt_node = undefined;
              selections = "alt";
              action = "deselect";
            } else if (selected_node != null) {
              selected_alt_node = d;
              selections = "alt";
              action = "select";
            } else {
              selected_node = d;
              selections = "main";
              action = "select";
            }
            const label_data = {
              selections: selections,
              action: action,
              styles: {
                "background-color" : selections == "alt" ? "grey" : "black",
                "color" : "white",
                "opacity" : 1
              }
            }
            console.log(label_data.action + ": " + label_data.selections);
            return(label_data);
          } else {
            return({ selections: "", action: "", styles: {} });
          }
        },
        (d : flat_branch) => {
          if(user_state === 'dividing' || user_state === 'auto-dividing') {
            if(selected_branch?.parent.name == d.parent.name && selected_branch?.child.name == d.child.name) {
              selected_branch = undefined;
            } else {
              selected_branch = d;
            }
            return({
              "stroke-dasharray" : "5px, 5px"
            });
          } else {
            return({});
          }
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

  <div id="main_view">
      <div id="tree_container">
      <svg id="tree" height={height} width={width + 100}>
        <rect width="100%" height="100%" fill="white"></rect>
        <g id="tree_g" transform="translate(10 0)"></g>
        <g id="x_axis" transform={`translate(10 ${0.96 * height})`}></g>
      </svg>
    </div>

    <div id="control_container">
      <div id="session_bar">
        <button onclick={() => {
          const svg = document.getElementById("tree");
          const anch = document.createElement("a");
          if(svg != null) {
            export_svg(svg as unknown as SVGElement, new OffscreenCanvas(1,1), anch);
          }
          document.removeChild(anch);
        }}>
          Export Image
        </button>
      </div>

      <div id="menu_bar">
        <button 
          class:menu_enabled={user_state === 'extruding'}
          onclick={() => user_state === 'extruding' ? user_state = 'base' : user_state = 'extruding'}
        >
          Extrude
        </button>
        <div class="button_group">
          <button 
            class:menu_enabled={user_state === 'dividing'}
            onclick={() => user_state === 'dividing' ? user_state = 'base' : user_state = 'dividing'}
          >
            Divide
          </button>
          <button 
            class:menu_enabled={user_state === 'auto-dividing'}
            onclick={() => user_state === 'auto-dividing' ? user_state = 'base' : user_state = 'auto-dividing'}
          >
            Auto-divide
          </button>
        </div>
        <div class="button_group">
          <button 
            class:menu_enabled={user_state === 'merging'}
            onclick={() => user_state === 'merging' ? user_state = 'base' : user_state = 'merging'}
          >
            Merge
          </button>
          <button 
            class:menu_enabled={user_state === 'auto-merging'}
            onclick={() => user_state === 'auto-merging' ? user_state = 'base' : user_state = 'auto-merging'}
          >
            Auto-merge
          </button>
        </div>
      </div>

      <div id="instruction_bar">
        {#if user_state === 'extruding'}
          <span>Select the node you would like to extrude.</span>
        {:else if user_state === 'dividing'}
          <span>Select the branch you would like to divide.</span>
        {:else if user_state === 'auto-dividing'}
          <span>Select branch you would like to automatically divide.</span>
        {:else if user_state === 'merging'}
          <span>Select the nodes you would like merge.</span>
        {:else if user_state === 'auto-merging'}
          <span>Press the button to begin auto-merge.</span>
        {/if}
      </div>


      {#if user_state === 'extruding' && selected_node != undefined}
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
      {:else if user_state === 'dividing' && selected_branch != undefined}
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
      {:else if user_state === 'auto-dividing' && selected_branch != undefined}
        <SelectionDialog
          selected={null}
          button_text={"Auto Divide"}
          button_action={
            () => auto_divide({ 
              node_name : parseInt((selected_branch as flat_branch).child.name),
            })
          }
        />
      {:else if user_state === 'merging' && selected_node != undefined && selected_alt_node != undefined}
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
      {:else if user_state === 'auto-merging'}
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
    /* border-color: rgb(212, 142, 29) !important;
    background-color: rgb(255, 236, 224) !important; */
    border-color: rgb(145, 10, 138) !important;
    background-color: rgb(241, 232, 240) !important;
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

  #menu_bar {
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

  .button_group {
    display: flex;
    flex-direction: row;
  }

  .button_group > button:first-child {
    border-right: none;
    border-top-right-radius: 0;
    border-bottom-right-radius: 0;
    border-top-left-radius: 0.2rem;
    border-bottom-left-radius: 0.2rem;
  }

  .button_group > button:last-child {
    border-top-left-radius: 0;
    border-bottom-left-radius: 0;
    border-top-right-radius: 0.2rem;
    border-bottom-right-radius: 0.2rem;
    border-right: 0.1rem solid black;
  }

  .button_group > button {
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

  .menu_enabled {
    /* background-color: black;
    color: white; */
    border-color: rgb(29, 29, 212);
    background-color: rgb(224, 235, 255);
  }

  .menu_enabled:active {
    /* background-color: black;
    color: white; */
    border-color: rgb(29, 29, 212);
    background-color: rgb(196, 215, 255);
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

  :global(body) {
    margin: 0;
  }
</style>