<script lang="ts">
  import katex from 'katex';
  import { untrack } from 'svelte';

  type ptype = {
    fullname : string,
    name_type : 'short' | 'latex',
    nameformula: string,
    dispname: string
  }
  let { name = $bindable() } : { name : ptype } = $props();

  let internal_formula = $state(name.name_type == "short" ? "" : name.nameformula);

  $effect(() => {
    if(internal_formula.length === 0) {
      untrack(() => name.name_type = 'short');
    } else {
      untrack(() => name.name_type = 'latex');
    }
  });

  $effect(() => {
    if(name.name_type === "short") {
      name.nameformula = name.fullname.split("_").map((part) => part[0]).join("");
      name.dispname = katex.renderToString("\\text{" + name.nameformula + "}", { output: 'html' });
    } else if (name.name_type === "latex") {
      name.nameformula = internal_formula;
      name.dispname = katex.renderToString(name.nameformula, { output: 'html', throwOnError: false });
    }
  })
</script>

<div id="editor">
  <div id="name_div">{name.fullname}</div>
  <div id="editor_div">
    <button 
      id="def_button"
      class:menu_enabled={name.name_type == 'short'}
      onclick={() => name.name_type = (name.name_type == 'short') ? 'latex' : 'short'}
    >default</button>
    <input id="formula_input" type="text" bind:value={internal_formula} placeholder="Type LaTeX formula to replace default." class:not_used={name.name_type === "short"}/>
  </div>
  <div id="render_div">{@html name.dispname}</div>
</div>

<style>
  #editor {
    display: flex;
    flex-direction: row;
    gap: 1rem;
    align-items: center;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
    padding: 0.5rem;

  }

  #name_div {
    width: 10rem;
    text-align: right;
  }

  #editor_div {
    display: flex;
    flex-direction: row;
  }

  #def_button {
    border-top-right-radius: 0;
    border-bottom-right-radius: 0;
    height: 2rem !important;
    box-sizing: border-box;
    margin: 0;
  }

  #formula_input {
    width: 15rem;
    border-top-left-radius: 0;
    border-bottom-left-radius: 0;
    border-top-right-radius: 0.2rem;
    border-bottom-right-radius: 0.2rem;
    border: 0.1rem solid black;
    height: 2rem !important;
    outline: none;
    box-sizing: border-box;
    border-left: none;
    margin: 0;
    padding: 0.5rem;
  }

  #render_div {
    width: 5rem;
  }

  .not_used {
    color: grey;
  }
</style>