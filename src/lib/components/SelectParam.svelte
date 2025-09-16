<script lang="ts">
  import { parse_index_strs, get_index_strs, rep_array } from "$lib/short_names";

  type props_t = { 
    name: string,
    indices: number[][],
    chosen_indices : number[][]
  }

  let { name, indices, chosen_indices = $bindable() } : props_t = $props();
  let error_state = $state(false);
  let is_selected = $derived(chosen_indices?.length > 0);

  $effect(() => {
    if(!is_selected) {
      error_state = false;
    }
  })
  
  let fixed_indices = $derived(indices.reduce((b, n) => b.map((bb, bi) => bb && n[bi] == indices[0][bi]), rep_array(true, indices[0].length) as boolean[]));
  let index_strings = $derived(get_index_strs(chosen_indices));

  function update_chosen() {
    try {
      let new_indices = parse_index_strs(index_strings);
      if(!valid_indices(new_indices)) {
        console.error(`Indices out of bounds!`);
        throw new Error("Indices out of bounds!");
      }
      chosen_indices = new_indices;
      error_state = false;
    } catch {
      error_state = true;
    }
  }

  function flip_selection() { 
    if(chosen_indices.length == 0) { 
      chosen_indices = indices;
    } else { 
      chosen_indices = [];
    }
  }

  function valid_indices(test_indices : number[][]) {
    return(test_indices.every((ti) => indices.some((ii) => arr_eq(ii, ti))));
  }

  function arr_eq(a1: number[], a2: number[]) {
    let aeq = true;
    a1.forEach((av, ai) => aeq = aeq && (av == a2[ai]));
    return aeq;
  }
</script>

<div id="container">
  <button id="param_name" class:is_selected={is_selected} onclick={flip_selection}>
    <span>{name}</span>
  </button>
  <div class = "sep"></div>
  <div id="index_inputs">
    {#if is_selected}
      <!-- <span class="bracket" class:error_state={error_state}>[</span> -->
      {#each index_strings as _, ii}
        {#if fixed_indices[ii]}
          <div class="fixed index_box" class:error_state={error_state}>{index_strings[ii]}</div>
        {:else}
          <input 
            type="text" class="index_box" bind:value={index_strings[ii]} 
            onkeyup={update_chosen}
            readonly={fixed_indices[ii]}
            class:error_state={error_state}
          />
        {/if}
      {/each}
      <!-- <span class="bracket" class:error_state={error_state}>]</span> -->
    {/if}
  </div>
</div>

<style>
  #container {
    display: flex;
    flex-direction: row;
    gap: 0.2rem;
  }

  button {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    gap: 0.5rem;
    width: 5rem;
    text-align: left;
    background: white;
    border-color: white;
    padding: 0.4rem;
  }

  button > span {
    display: block;
    overflow-x: hidden;
  }

  .is_selected {
    /* background: black;
    color: white; */
    border-color: rgb(29, 29, 212);
    background-color: rgb(224, 235, 255);
  }

  /* .is_selected:hover {
    background: black;
    color: white;
    border-color: rgb(29, 29, 212);
    background-color: rgb(196, 215, 255);
  } */

  /* button:hover {
    background: rgb(234, 234, 234);
  } */

  .bracket {
    font-size: 1.1rem;
  }

  #index_inputs {
    display: flex;
    flex-direction: row;
    gap: 0rem;
  }

  .index_box {
    width: 3rem;
    text-align: center;
    /* background: rgb(243, 243, 243); */
  }

  .index_box {
    border: none;
    border-top: 0.2rem solid grey;
    border-bottom: 0.2rem solid grey;
  }

  .index_box:only-child {
    border-radius: 0.2rem !important;
    border: 0.1rem solid grey !important;
  }

  .index_box:first-child {
    border: none;
    border-radius: none;
    border-top-left-radius: 0.2rem;
    border-bottom-left-radius: 0.2rem;
    border: 0.1rem solid grey;
    border-right: none;
  }

  .index_box:last-child {
    border: none;
    border-radius: none;
    border-top-right-radius: 0.2rem;
    border-bottom-right-radius: 0.2rem;
    border: 0.1rem solid grey;
    border-left: none;
  }

  input[type="text"] {
    appearance: none;
    border: none;
  }

  .fixed {
    display: flex;
    justify-content: center;
    align-items: center;
    vertical-align: middle;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.8rem;
    user-select: none;
    color: grey;
  }

  input[type="text"]:focus {
    outline: none;
    /* border-bottom: 0.1rem solid blue; */
  }

  .error_state {
    color: red;
  }

  /* .sep {
    border-right: 0.1rem solid black;
  } */
</style>