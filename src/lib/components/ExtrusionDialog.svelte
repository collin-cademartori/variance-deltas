<script lang="ts">
  import { type flat_node } from "../tree";
  import { by_parameter } from "$lib/short_names";
  import { extrude_branch } from "$lib/tree_methods";

  import SelectParam from "./SelectParam.svelte";

  let { selected } : { selected : flat_node } = $props();

  let selected_params = $derived(by_parameter(selected.params));

  let chosen : number[][][] = $state(selected_params.map(() => []));

  $effect(() => {
    chosen = selected_params.map(() => []);
  })

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

  function extrude() {
    console.log(chosen_strings());
    extrude_branch({
      node_name: parseInt(selected.name),
      params_kept: chosen_strings()
    });
  }
</script>

<div id="dialog">
  <div id="dialog_menu">
    <button class="dialog_button" onclick={select_all}>Select all</button>
    <button class="dialog_button" onclick={deselect_all}>Deslect all</button>
  </div>
  <div id="param_list">
    {#each selected_params as param, pi}
      <SelectParam name={param.name} indices={param.indices} bind:chosen_indices={chosen[pi]} />
    {/each}
  </div>
  <button id="submit_button" onclick={extrude}>
    Extrude
  </button>
</div>

<style>
  #dialog {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
    padding: 1rem;
    border: 0.1rem solid darkgrey;
    width: 16rem;
  }

  #param_list {
    display: flex;
    flex-direction: column;
    gap: 0.2rem;
  }

  #submit_button {
    margin-top: 1rem;
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