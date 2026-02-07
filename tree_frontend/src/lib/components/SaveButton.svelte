<script lang="ts">
  import { save_state } from "$lib/tree_methods";
  import { handle_save } from "$lib/websocket.svelte";
  import { user_state } from "$lib/state/user_state.svelte";

  let save_dialog : HTMLDialogElement;
  let save_input : HTMLInputElement;

  handle_save((succ : boolean) => {
    if(succ) {
      saving = false;
    } else {
      saving = false;
      failed = true;
    }
  });

  function is_key_event(e : KeyboardEvent | MouseEvent): e is KeyboardEvent {
    return(e.type === "keydown");
  }

  function run_save(e : KeyboardEvent | MouseEvent, fname? : string) {
    if(!user_state.fname && fname && fname.length > 0) {
      user_state.fname = fname;
    }
    if(!is_key_event(e) || e.key === "Enter") {
      if(user_state.fname) {
        saving = true;
        failed = false;
        save_dialog.close();
        save_state({"fname": user_state.fname});
      } else {
        save_dialog.show();
      }
    }
  }

  let saving = $state(false);
  let failed = $state(false);
</script>

<dialog id="save_name" bind:this={save_dialog}>
  <div id="save_name_container">
    <input 
      type="text" id="save_name_input" placeholder="File Name"
      bind:this={save_input}
      onkeydown={(e) => run_save(e, save_input.value)}
    />
    <button 
      id="save_name_button" onclick={(e) => run_save(e, save_input.value)}
    >Save</button>
  </div>
</dialog>

<div id="save_container">
  <button id="save_button" onclick={run_save}>Save</button>
  {#if failed}
    <span id="failed_indicator">
      Failed to save!
    </span>
  {:else if saving}
    <span id="saving_indicator">
      Saving...
    </span>
  {/if}
</div>

<style>
  #save_container {
    display: flex;
    flex-direction: row;
    gap: 0.5rem;
    align-items: center;
    height: 3rem;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  }

  #save_button {
    height: fit-content;
  }

  #saving_indicator {
    color: grey;
    font-size: 0.9rem;
  }

  #failed_indicator {
    color: darkred;
    font-size: 0.9rem;
  }

  #save_name {
    height: fit-content;
    width: 20rem;
    background: white;
    overflow-y: hidden;
    border: 0.15rem solid black;
    border-radius: 0.2rem;
    box-shadow: rgb(57, 57, 57) 0 1px 25px;
    margin-top: 5rem;
  }

  #save_name_container {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
  }

  #save_name_input {
    width: 100%; 
    box-sizing: border-box;
  }

  #save_name_button {
    width: fit-content;
    margin-left: auto;
  }
</style>