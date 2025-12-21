<script lang="ts">
  import { user_state } from "$lib/user_state.svelte";

  let { show_dialog = $bindable() } : { show_dialog : boolean } = $props();

  let dialog : HTMLDialogElement;
  
  $effect(() => {
    if(show_dialog) {
      dialog.showModal();
    } else {
      dialog.close();
    }
  })
</script>

<dialog id="names_dialog" bind:this={dialog}>
  <div id="dialog_div">
    <div id="names_bar">
      <div id="names_bar_title">
        <span>
          Edit Quantity Names
        </span>
      </div>
      <button 
        id="close_button"
        onclick={() => {
          show_dialog = false;
        }}
      >
        Close
      </button>
    </div>

    <div id="editor_container">

    </div>
  </div>
</dialog>

<style>

  #names_dialog {
    height: 85%;
    width: 75%;
    background: white;
    overflow-y: hidden;
  }

  #dialog_div {
    display: flex;
    flex-direction: column;
    gap: 1rem;
    overflow-y: hidden;
    height: 100%;
  }

  #editor_container {
    overflow-y: scroll;
    height: 100%;
    border: 1px solid darkgrey;
  } 

  #names_bar {
    display: flex;
    flex-direction: row;
    gap: 0.5rem;
    justify-content: left;
  }

  #names_bar_title {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: left;
    justify-content: center;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
  }

  button {
    width: fit-content;
  }
</style>