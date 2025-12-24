<script lang="ts">
  import { user_state } from "$lib/state/user_state.svelte";
  import { remove_group } from "$lib/state/groups";
  
  let { group_name } : { group_name : string } = $props();

  function set_group() {
    if(user_state.group === group_name) {
      user_state.group = undefined;
    } else {
      user_state.group = group_name;
    }
  }
</script>

<div id="item">
  <div id="item_name">
    {group_name}
  </div>
  <button onclick={set_group} class:menu_enabled={user_state.group === group_name}>
    View
  </button>
  <button
    onclick={() => {
      if(user_state.group == group_name) {
        user_state.group = undefined;
      }
      remove_group(group_name);
    }}
  >
    Del
  </button>
</div>

<style>
  #item {
    display: flex;
    flex-direction: row;
    align-items: center;
    gap: 0.2rem;
    /* border: 0.1rem solid black; */
    border: none;
    border-radius: 0.2rem;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    padding: 0.4rem;
    padding-left: 0.4rem;
    padding-right: 0.4rem;
    user-select: none;
  }

  /* button {
    background: rgb(233, 233, 233);
    border: none;
  } */

  #item_name {
    width: 100%;
    overflow-x: hidden;
  }
</style>