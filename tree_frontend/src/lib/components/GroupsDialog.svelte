<script lang="ts">
  import { groups } from "$lib/state/groups";
  import GroupItem from "./GroupItem.svelte";
  import { user_state } from "$lib/state/user_state.svelte";
  import { selection } from "$lib/state/selection.svelte";
  import MultiSelectionDialog from "./MultiSelectionDialog.svelte";
  import { make_group } from "$lib/state/groups";
  import { slide } from "svelte/transition";
</script>

<div id="dialog">
  <div id="dialog_title" class:alone={groups.size === 0 && user_state.state !== 'add-group'}>
    <span>Node Groups</span>
    <button
      id="add_group"
      class:menu_enabled={user_state.state === 'add-group'}
      onclick={() => user_state.state === 'add-group' ? user_state.state = 'base' : user_state.state = 'add-group'}
    >
      Add Group
    </button>
  </div>
  {#if user_state.state === "add-group"}
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
  {:else if groups.size > 0}
    <div id="group_item_container" transition:slide={{ duration: 200 }}>
      {#each groups.keys() as group_name}
        <GroupItem {group_name} />
      {/each}  
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

  #add_group {
    margin-left: auto;
  }

  #group_item_container {
    display: flex;
    flex-direction: column;
    padding: 1rem 1rem 1rem 1rem;
  }
</style>