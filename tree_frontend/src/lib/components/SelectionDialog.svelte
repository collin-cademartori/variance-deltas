<script lang="ts">
  import type { flat_node } from "../state/types";
  import { by_parameter } from "$lib/state/short_names";
  import { slide } from "svelte/transition";
  import { user_state } from "$lib/state/user_state.svelte";

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

<div id="dialog" transition:slide={{ duration: 200 }}>
  {#if selected_params.length > 0}
    <div id="param_list">
      <span>Include:</span>
      {#each selected_params as param, pi}
        <SelectParam name={user_state.names.get(param.name)?.formatted_name ?? param.name} indices={param.indices} bind:chosen_indices={chosen[pi]} />
      {/each}
    </div>
  {/if}
  <div id="dialog_menu">
    <button class="dialog_button" onclick={select_all}>Select all</button>
    <button class="dialog_button" onclick={deselect_all}>Deslect all</button>
    <button id="submit_button" onclick={() => button_action(chosen_strings())}>
      {button_text}
    </button>
  </div>
</div>

<style>
  #dialog { 
    box-sizing: border-box;
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    border-radius: 0.2rem;
    width: 100%;
    /* border: 0.1rem solid rgb(106, 106, 106); 
    box-shadow: 0rem 0.1rem 0.3rem 0rem rgb(213, 213, 213);
    background: white; */
  }

  #param_list {
    display: flex;
    flex-direction: row;
    align-items: center;
    flex-wrap: wrap;
    width: 100%;
    gap: 0.5rem;
  }

  #param_list > span {
    font-size: 0.8rem;
  }

  #dialog_menu {
    display: flex;
    flex-direction: row;
    justify-content: right;
  }

  .dialog_button {
    border: none;
    background: none;
    padding-left: 0;
    padding-right: 0.5rem;
  }

  .dialog_button:hover {
    background: none;
    text-decoration: underline;
  }

  .dialog_button:active {
    background-color: none;
    text-decoration: underline;
  }

  #submit_button {
    width: fit-content;
    margin-left: auto;
  }
</style>