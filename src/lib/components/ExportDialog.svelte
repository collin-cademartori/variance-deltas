<script lang="ts">
  import { user_state } from "$lib/user_state.svelte";
  import { export_svg } from "$lib/export_svg";
  import { draw_tree } from "$lib/draw_tree";
  import { groups } from "$lib/groups";
  import { scaleLinear, axisBottom, select } from "d3";

  let { show_dialog = $bindable(), plot_width, y } : { show_dialog : boolean, plot_width : number, y : d3.ScaleLinear<number, number, never> } = $props();

  let dialog : HTMLDialogElement;
  let svg_snapshot : SVGElement | null = null;
  let xaxis_g : SVGGElement;

  function do_export() {
    const anch = document.createElement("a");
    if(svg_snapshot != null) {
      export_svg(svg_snapshot as unknown as SVGElement, new OffscreenCanvas(1,1), anch);
    }
    document.removeChild(anch);
  }
  
  $effect(() => {
    if(show_dialog) {
      dialog.showModal();

      if(user_state.tree != null) {
        const cur_group = groups.get(user_state.group ?? "");
        const flat_tree = [...user_state.tree]
          .map((node) => node.data)
          .filter((node) => cur_group ? cur_group.has(node.name) : true);
        const max_y = flat_tree.map((tnode) => tnode.x_pos).reduce((p,n) => Math.max(p ?? 0, n ?? 0)) ?? 0;
        const snapshot_height = y(max_y ?? 0) + y(0.1) + 36;
        svg_snapshot?.setAttribute('height', snapshot_height.toString());
        xaxis_g.setAttribute('transform', `translate(10 ${0.95 * snapshot_height})`);
        const x = scaleLinear([0, 1], [0, 0.95 * plot_width]);
        const xaxis = axisBottom(x).offset(10).tickPadding(15).tickSize(0);
        xaxis(select("#x_axis_static"));
        draw_tree(
          flat_tree,
          x,
          y,
          36, () => null, () => null, document.styleSheets[0],
          true
        );
      }
    } else {
      dialog.close();
    }
  })
</script>

<dialog id="export_dialog" bind:this={dialog}>
  <div id="dialog_div">
    <div id="export_bar">
      <div id="preview_image">
        <span>
          Preview Image
        </span>
      </div>
      <button 
        id="export_button"
        onclick={() => {
          show_dialog = false;
          do_export();
        }}
      >
        Export
      </button>
      <button 
        id="close_button"
        onclick={() => {
          show_dialog = false;
        }}
      >
        Close
      </button>
    </div>

    <div id="snapshot_container">
      {#if show_dialog}
        <svg id="tree_static" width={plot_width + 100} bind:this={svg_snapshot}>
          <rect width="100%" height="100%" fill="white"></rect>
          <g id="tree_g_static" transform="translate(10 0)">
            <rect width="100%" height="100%" fill="white"></rect>
          </g>
          <g id="x_axis" bind:this={xaxis_g}>
            <g id="x_axis_static"></g>
          </g>
        </svg>
      {/if}
    </div>
  </div>
</dialog>

<style>

  #export_dialog {
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

  #snapshot_container {
    overflow-y: scroll;
    height: 100%;
    border: 1px solid darkgrey;
  } 

  #export_bar {
    display: flex;
    flex-direction: row;
    gap: 0.5rem;
    justify-content: left;
  }

  #preview_image {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: left;
    justify-content: center;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 0.9rem;
  }

  #preview_image > span {
    display: block;
  }

  button {
    width: fit-content;
  }
</style>