<script lang="ts">
  import { user_state } from "$lib/state/user_state.svelte";
  import { SvelteMap } from "svelte/reactivity";
  import NameEditor from "./NameEditor.svelte";
  import { onMount, untrack } from "svelte";

  type ptype = {
    fullname : string,
    name_type : 'short' | 'latex',
    nameformula: string,
    dispname: string
  }

  let { show_dialog = $bindable() } : { show_dialog : boolean } = $props();

  let dialog : HTMLDialogElement;

  let names : Array<ptype> = $state([]);
  
  $effect(() => {
    if(show_dialog) {
      untrack(reset_names);
      dialog.showModal();
    } else {
      dialog.close();
    }
  });

  function close_dialog() {
    user_state.names = new SvelteMap(names.map((namedata) => {
      return([namedata.fullname, {
        type: namedata.name_type,
        name: namedata.nameformula,
        formatted_name: namedata.dispname
      }]);
    }))
    show_dialog = false;
  }

  function reset_names() {
    names.length = 0;
    [...user_state.names.entries()].forEach(([fname, name_data]) => { 
        names.push({
          fullname: fname,
          name_type: name_data.type,
          dispname: name_data.formatted_name,
          nameformula: name_data.name
        });
    });
  }
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
        onclick={reset_names}
      >
        Reset
      </button>
      <button 
        id="close_button"
        onclick={close_dialog}
      >
        Close
      </button>
    </div>

    <div id="editor_container">
      {#each names as name, i}
        {#if name.fullname != "__globals__"}
          <NameEditor bind:name={names[i]}/>
        {/if}
      {/each}
    </div>
  </div>
</dialog>

<style>

  #names_dialog {
    background: white;
    overflow-y: hidden;
    border: 0.15rem solid black;
    border-radius: 0.2rem;
    box-shadow: rgb(57, 57, 57) 0 1px 25px;
  }

  #dialog_div {
    display: flex;
    flex-direction: column;
    gap: 1rem;
    overflow-y: hidden;
    height: 100%;
  }

  #editor_container {
    overflow-y: auto;
    height: 100%;
    border: 1px solid lightgrey;
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