<script lang="ts">
  import { user_state } from "$lib/state/user_state.svelte";
  import { export_svg } from "$lib/export_svg";
  import { draw_tree } from "$lib/state/draw_tree";
  import { groups } from "$lib/state/groups";
  import { make_config } from "$lib/state/draw_data";
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
    anch.remove();
  }
  
  $effect(() => {
    if(show_dialog) {
      dialog.showModal();

      const katex_css = (document.getElementById("katex_style") as HTMLLinkElement).sheet;
      const svg_css = document.getElementById("export_styles");
      if(katex_css != null && svg_css != null) {
        const css_str = [...katex_css.cssRules].reduce((psheet, rule) => psheet + "\n" + rule.cssText, "");
        svg_css.innerHTML = css_str;
      } else {
        console.error("BAD STYLES")
      }

      if(user_state.tree != null) {
        const cur_group = groups.get(user_state.group ?? "");
        const flat_tree = [...user_state.tree]
          .map((node) => node.data)
          .filter((node) => cur_group ? cur_group.has(node.name) : true);
        const max_y = flat_tree.map((tnode) => tnode.x_pos).reduce((p,n) => Math.max(p ?? 0, n ?? 0)) ?? 0;
        const snapshot_height = y(max_y ?? 0) + y(0.1) + 2 * 36;
        svg_snapshot?.setAttribute('height', snapshot_height.toString());
        xaxis_g.setAttribute('transform', `translate(10 ${snapshot_height - 30})`);
        const x = scaleLinear([0, 1], [0, 0.95 * plot_width]);
        const xaxis = axisBottom(x).offset(10).tickPadding(15).tickSize(0);
        xaxis(select("#x_axis_static"));
        select("#x_axis_static").attr("font-size", (50 * (12 / 36)) + "px");
        draw_tree(
          flat_tree, 
          {x: x, y: y},
          { limit: user_state.global_limit, params: user_state.globals },
          make_config({
            label_height: 50,
            show_globals: user_state.show_globals,
            draw_color: "black",
            highlight_color: "black",
            draw_static: true,
          }),
          { branch_select: () => {}, node_select: () => {}, node_hover: () => {}, node_unhover: () => {}},
          user_state.names
        );
      }
    } else {
      const svg_css = document.getElementById("export_styles");
      if (svg_css != null) {
        svg_css.innerHTML = "";
      }
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
        <svg id="tree_static" width={plot_width + 200} bind:this={svg_snapshot}>
          <style id="export_styles"></style>
          <rect width="100%" height="100%" fill="white"></rect>
          
          <g id="tree_outer_static" transform="translate(20 40)">
            <rect id="global_limit_rect_st  atic" 
              y="-10" x="100%" 
              fill="#eeeeee" stroke="black"
              opacity="0.3"
              stroke-dasharray="8 8"
              stroke-width="1px"
              height="110%" width="100%">
            </rect>
            <g id="tree_layers_static">
              <g id="tree_g_static"></g>
              <g id="tree_g_del_static"></g>
              <g id="tree_g_alt_static"></g>
              <g id="tree_g_main_static"></g>
            </g>
          </g>

          <g id="label_layer_container_static" transform="translate(20 40)">
            <g id="label_layer"></g>
          </g>
          
          <g id="x_axis_container" bind:this={xaxis_g}>
            <rect width="100%" height="60" fill="white"></rect>
            <g id="x_axis_static"></g>
          </g>

          <g id="top_bar_static" transform="translate(20 0)">
            <rect fill="white" height="40" width="100%" transform="translate(-20 0)"></rect> 
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