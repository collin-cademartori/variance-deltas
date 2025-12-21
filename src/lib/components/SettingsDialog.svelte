<script lang="ts">
  import NamesDialog from "./NamesDialog.svelte";
  import { user_state } from "$lib/user_state.svelte";

  let show_names = $state(false)
</script>

<div id="dialog">
  <span>Layout type</span>
  <div class="button_group" id="layout_type">
    <button 
      class:menu_enabled={user_state.layout_format === 'long'}
      onclick={() => user_state.layout_format = 'long'}
    >
      Hierarchy 
    </button>
    <button 
      class:menu_enabled={user_state.layout_format === 'normal'}
      onclick={() => user_state.layout_format = 'normal'}
    >
      Tree
    </button>
  </div>

  <span>Show global parameters</span>
  <div class="button_group" id="layout_type">
    <button 
      class:menu_enabled={user_state.show_globals === true}
      onclick={() => user_state.show_globals = true}
    >
      Show 
    </button>
    <button 
      class:menu_enabled={user_state.show_globals === false}
      onclick={() => user_state.show_globals = false}
    >
      Hide
    </button>
  </div>

  <span>Quantity Names</span>
  <button 
      onclick={() => show_names = true}
    >
      Edit Names
    </button>
  <!-- <div class="button_group" id="layout_type">
    <button 
      class:menu_enabled={user_state.show_globals === true}
      onclick={() => user_state.show_globals = true}
    >
      Edit 
    </button>
  </div> -->
</div>

<NamesDialog bind:show_dialog={show_names}/>

<style>
  #dialog {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    padding: 1rem;
    border: 0.1rem solid black;
    border-radius: 0.2rem;
    width: 16rem;
  }

  #layout_type {
    width: 100%;
  }

  #layout_type > button {
    width: 50%;
  }

  span {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
  }
</style>