<script lang="ts">
  import type { flat_node } from "../state/types";
  import { by_parameter } from "$lib/state/short_names";

  import SelectParam from "./SelectParam.svelte";

  let { selected = null, button_text, button_action } : { selected : flat_node | null, button_text : string, button_action : (params : string[]) => void } = $props();

  let selected_params = $derived(selected === null ? [] : by_parameter(selected.params));

  let chosen : number[][][] = $state([]);

  $effect(() => { chosen = selected_params.map(() => []) });

  const chosen_strings = () => {
    return(chosen.map((indices, pos) => indices.map((index) => selected_params[pos].name + "[" + index + "]"))
                 .reduce((p, n) => p.concat(n), []));
  };

  function select_all() {
    chosen = selected_params.map((sp) => sp.indices);
  }

  function deselect_all() {
    chosen = selected_params.map(() => []);
  }
</script>

<div id="dialog">
  {#if selected_params.length > 0}
    <div id="dialog_menu">
      <button class="dialog_button" onclick={select_all}>Select all</button>
      <button class="dialog_button" onclick={deselect_all}>Deslect all</button>
    </div>
    <div id="param_list">
      {#each selected_params as param, pi}
        <SelectParam name={param.name} indices={param.indices} bind:chosen_indices={chosen[pi]} />
      {/each}
    </div>
  {/if}
  <button id="submit_button" onclick={() => button_action(chosen_strings())}>
    {button_text}
  </button>
</div>

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

  #param_list {
    display: flex;
    flex-direction: column;
    gap: 0.2rem;
  }

  .dialog_button {
    border: none;
  }

  .dialog_button:hover {
    background: none;
    text-decoration: underline;
  }

  .dialog_button:active {
    background-color: none;
    text-decoration: underline;
  }
</style>