<script lang="ts">
  import type { flat_node, flat_branch } from "$lib/state/types";
  import { user_state } from "$lib/state/user_state.svelte";
  import { selection } from "$lib/state/selection.svelte";
  import SelectionDialog from "./SelectionDialog.svelte";
  import { extrude_branch, delete_node, merge_nodes, divide_branch, auto_divide, auto_merge } from "$lib/tree_methods";
  import { slide } from "svelte/transition"

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

  let editing = $derived(['extruding', 'deleting', 'dividing', 'auto-dividing', 'merging'].includes(user_state.state));
</script>

<div id="dialog">
  <div id="dialog_title" 
       class:alone={!editing}>
    <span>Edit Tree</span>
    <div class="button_group" id="edit_actions">
      <button 
          class:menu_enabled={user_state.state === 'extruding'}
          onclick={() => user_state.state === 'extruding' ? user_state.state = 'base' : user_state.state = 'extruding'}
        >
          Extrude
        </button>
        <button 
          class:menu_enabled={user_state.state === 'dividing'}
          onclick={() => user_state.state === 'dividing' ? user_state.state = 'base' : user_state.state = 'dividing'}
        >
          Divide
        </button>
        <button 
          class:menu_enabled={user_state.state === 'merging'}
          onclick={() => user_state.state === 'merging' ? user_state.state = 'base' : user_state.state = 'merging'}
        >
          Merge
        </button>
        <button 
          class:menu_enabled={user_state.state === 'deleting'}
          onclick={() => user_state.state === 'deleting' ? user_state.state = 'base' : user_state.state = 'deleting'}
        >
          Delete
        </button>
    </div>
  </div>

  {#if editing}
  <div id="editor_content">
    <div id="instruction_bar" transition:slide={{ duration: 200 }}>
      {#if user_state.state === 'extruding' && selected_node == undefined}
        <span>Select the node you would like to extrude.</span>
      {:else if user_state.state === 'deleting' && selected_node == undefined}
        <span>Select the node you would like to delete.</span>
      {:else if user_state.state === 'dividing' && selected_branch.parent == undefined}
        <span>Select the branch you would like to divide.</span>
      {:else if user_state.state === 'auto-dividing' && selected_branch.parent == undefined}
        <span>Select branch you would like to automatically divide.</span>
      {:else if user_state.state === 'merging' && (selected_alt_node == undefined)}
        <span>Select the nodes you would like merge.</span>
      {/if}
    </div>

    {#if user_state.state === 'extruding' && selected_node != undefined}
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
    {/if}
  </div>
  {/if}
</div>

<style>
  #dialog {
    box-sizing: border-box;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
    display: flex;
    flex-direction: column;
    /* gap: 1rem; */
    border-radius: 0.2rem;
    width: 22rem;
    border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    background: white;
  }

  #dialog_title {
    /* margin-bottom: 1rem; */
    display: flex;
    flex-direction: row;
    border-bottom: 0.1rem solid rgb(152, 152, 152);
    padding-left: 1rem;
    padding-right: 1rem;
    padding-bottom: 1rem;
    padding-top: 1rem;
    align-items: center;
    background-color: rgb(253, 253, 253);
    /* background: linear-gradient(180deg, rgb(255,255,255), rgb(252, 252, 252)) */
    transition: border-bottom 0.1s linear;
  }

  #dialog_title.alone {
    border-bottom: 0rem solid white;
  }

  #editor_content {
    padding: 1rem;
  }

  #instruction_bar {
    font-size: 0.8rem;
    padding: 0 1.5rem 0 1.5rem;
    /* text-align: center; */
    color: rgb(77, 77, 77);
  }

  #edit_actions {
    margin-left: auto;
  }
</style>