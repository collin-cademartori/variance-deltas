<script lang="ts">
  import NamesDialog from "./NamesDialog.svelte";
  import { user_state } from "$lib/state/user_state.svelte";

  let show_names = $state(false)
</script>

<div id="dialog">
  <div id="dialog_title">
    <span>Settings</span>
  </div>
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
    <span>Globals parameters</span>
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
    box-sizing: border-box;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
    display: flex;
    flex-direction: column;
    padding: 0rem 0 0.5rem 0;
    border-radius: 0.2rem;
    width: 22rem;
    border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    background: white;
  }

  #dialog_title {
    margin-bottom: 1rem;
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
  }

  #dialog_title > span {
    font-size: 0.9rem;
  }

  .option_container {
    display: flex;
    flex-direction: row;
    /* gap: 0.5rem; */
    padding: 0 1rem 0.5rem 1rem;
    align-items: center;
  }

  .option_container > span {
    margin-right: auto;
  }

  /* #layout_type {
    width: 100%;
  } */

  #layout_type > button {
    width: 5rem;
  }

  span {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
  }
</style>