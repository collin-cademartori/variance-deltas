<script lang="ts">
  import { type flat_node } from "../state/types";
  import { selector, selection } from "$lib/state/selection.svelte";
  import { user_state } from "$lib/state/user_state.svelte";

  type props_t = { 
    selected : Array<flat_node | undefined> | null,
      input_text : string,
      button_text : string,
      button_action : (name: string, node_names : string[]) => void 
  };

  let { selected = null, input_text, button_text, button_action } : props_t = $props();

  let user_text : string = $state("");

  let node_names = $derived.by(() => {
    const sel_names = selected?.map((node) => node?.name ?? "");
    if(selector.type === 'anc') {
      return(sel_names ?? null);
    } else if (selector.type === 'desc') {
      if(user_state.tree && selected) {
        return([...user_state.tree]
                .filter((node) => !sel_names?.includes(node.data.name))
                .map((node) => node.data.name)
              );
      } else {
        return(null);
      }
    } else {
      return(null);
    }
  });

</script>

<div id="dialog">
  <button 
    disabled={selected == null || selected.length == 0}
    onclick={() => {
      selection.clear();
    }}
  >
    Clear Selection
  </button>
  <div class="button_group" id="sel_type">
    <button 
      class:menu_enabled={selector.type === 'anc'}
      onclick={() => selector.type = 'anc'}
    >
      Include Nodes
    </button>
    <button 
      class:menu_enabled={selector.type === 'desc'}
      onclick={() => selector.type = 'desc'}
    >
      Exclude Nodes
    </button>
  </div>
  <label for="text_input">{input_text}</label>
  <input type="text" name="text_input" bind:value={user_text} />
  <button 
    id="submit_button" 
    disabled={selected == null || selected.length == 0}
    onclick={() => button_action(user_text, node_names ?? [])}
  >
    {button_text}
  </button>
</div>

<style>
  label {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
  }

  input[type="text"] {
    border: 0.1rem solid black;
    border-radius: 0.2rem;
    padding: 0.2rem;
  }

  #dialog {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    padding: 1rem;
    border: 0.1rem solid black;
    border-radius: 0.2rem;
    width: 16rem;
  }

  #sel_type {
    width: 100%;
  }
</style>