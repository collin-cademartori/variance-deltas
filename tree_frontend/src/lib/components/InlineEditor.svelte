<script lang='ts'>
    import { tick } from "svelte";
    import type { HTMLInputTypeAttribute } from "svelte/elements";

  let { title = $bindable() } : { title : string | undefined } = $props();

  let editing = $state(false);
  let text_in : undefined | HTMLInputElement = $state(undefined);

  function set_title(e : KeyboardEvent) {
    if(e.currentTarget && e.key === "Enter") {
      const new_title = (e.currentTarget as HTMLInputElement).value;
      title = new_title;
      editing = false;
    }
  }

  async function enable_editing() {
    if(!editing) {
      editing = true;
    }
    await tick();
    if(text_in) {
      text_in.focus();
    }
  }
</script>

<div id="session_title" onclick={enable_editing}>
  {#if editing}
    <input type="text" value={title ?? ""} onkeydown={set_title} bind:this={text_in} id="session_title_input"/>
  {:else}
    <span id="session_title_span">{title ?? "Untitled Project"}</span>
  {/if}
</div>

<style>
  #session_title {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    /* border-bottom: 0.1rem solid black; */
    padding: 0.5rem;
    width: 100%;  
    user-select: none;
    max-width: 15rem;
    overflow-x: clip;
    width: fit-content;
    display: flex;
    align-items: center;
  }

  #session_title_span {
    text-decoration: underline;
    font-size: 1.2rem;
    white-space: nowrap;
  }

  #session_title_input {
    font-size: 1.2rem;
    max-width: 14rem;
  }

  
</style>