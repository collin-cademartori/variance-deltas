<script lang="ts">
  import NamesDialog from "./NamesDialog.svelte";
  import { user_state } from "$lib/state/user_state.svelte";

  let show_names = $state(false)
</script>

<div id="dialog">
  <div class="option_container">
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
  </div>

  <div class="option_container">
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
  </div>

  <div class="option_container">
    <span>Quantity Names</span>
    <button 
      onclick={() => show_names = true}
    >
      Edit Names
    </button>
  </div>
</div>

<NamesDialog bind:show_dialog={show_names}/>

<style>
  #dialog {
    display: flex;
    flex-direction: column;
    gap: 1rem;
    padding: 1.5rem;
    border-radius: 0.2rem;
    width: 16rem;
    border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    background: white;
  }

  .option_container {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
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