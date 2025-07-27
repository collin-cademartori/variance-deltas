<script lang="ts">
  import { onMount } from "svelte";
  import * as ws from "$lib/websocket";
  import * as d3 from "d3";
  import { annotate_tree, type flat_node, type flat_tree, type flat_branch } from "$lib/test_tree";
  import { divide_branch, extrude_branch, get_tree, reset_tree } from "$lib/tree_methods";

  const height = 500;
  const width = 1500;

  const x = d3.scaleLinear([0, 1], [0, width]);
  const y = d3.scaleLinear([0, 1], [0, 0.95 * height]);
  const l_height = 0.045;

  function draw_tree(tree : flat_tree, wf : (text : string | undefined) => number) {
    let tree_elem = d3.select("#tree_g");

    let branch_data = tree.filter((n) => n.parent != "").map((n) => {
      const pn = tree.find((n2) => n2.name === n.parent);
      if(pn != null) {
        return {
          child: n,
          parent: pn
        }
      } else {
        throw new Error("Cannot find parent! Branch data failed.")
      }
    });

    console.log(branch_data);

    const link = d3.line<flat_node>(
      (d) => x(d.ered),
      (d) => y(d.x_pos ?? 0)
    ).curve(d3.curveStepBefore);

    tree_elem.selectAll(".tree_branch")
             .data(branch_data, (d) => `${(d as flat_branch).parent.name}-->${(d as flat_branch).child.name}`)
             .join((enter) => {
                let g = enter.append("g").attr("class", "tree_branch");
                g.transition().attr("opacity", 1);

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                     .attr("class", "branch_path")
                     .attr("stroke", "black")
                     .attr("stroke-width", 2.5)
                     .attr("fill", "none")
                     .attr("id", (d) => `branch-${d.child.shortname}-${d.parent.shortname}`);

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                         .attr("class", "select_path")
                         .attr("stroke", "red")
                         .attr("stroke-width", 15)
                         .attr("fill", "none")
                         .attr("opacity", 0)
                         .on("mouseover", (ev, d) => {
                           const vis_path = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`);
                           vis_path?.setAttribute("stroke", "#0161df");
                         })
                         .on("mouseleave", (ev, d) => {
                           const vis_path = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`);
                           vis_path?.setAttribute("stroke", "black");
                         })
                         .on("click", (ev, d) => {
                           divide_branch({
                            node_name: parseInt(d.child.name),
                            params_kept: d.parent.params[9]
                           })
                         });

                return g;
             },
            (update) => {
              update.select(".branch_path").transition()
                     .attr("d", (d) => link([d.parent, d.child]))
              update.select(".select_path").transition()
                     .attr("d", (d) => link([d.parent, d.child]))
              return update;
            },
            (exit) => {
              exit.transition().attr("opacity", 0).remove()
            }
          );

    tree_elem.selectAll(".tree_node")
      // .data(tree, (d) => d ? (d as flat_node).name : this.id)
      .data(tree, (d) => (d as flat_node).name)
      .join((enter) => {
        let g = enter.append("g")
                     .attr("id", (d) => `${d.name}`)
                     .attr("class", "tree_node");

        g.transition().attr("opacity", 1);

        g.append("circle")
          .attr("r", 5)
          .attr("cx", (d : flat_node) => x(d.ered))
          .attr("cy", (d: flat_node) => y(d.x_pos ?? 0))
          .attr("fill", "black")
          //.attr("stroke", "white")
          .attr("stroke-width", 2.5);

        let fo = g.append("foreignObject")
          .attr("class", "label_fo")
          .attr("x", (d : flat_node) => x(0.003 + d.ered))
          .attr("y", (d: flat_node) => y(d.label_y ?? 0))
          .attr("height", (d: flat_node) => y(l_height))
          .attr("width", (d: flat_node) =>  x(2 * wf(d.shortname)));

        let ld = fo.append("xhtml:div")
          .attr("class", "label-div")
          .style("background", "white")
          .style("font-family", "sans-serif")
          .style("box-sizing", "border-box")
          .style("padding", "4px")
          .style("border-width", "0.1rem")
          .style("border-color", "black")
          .style("border-style", "solid")
          //.style("border-radius", "0.2rem")
          .style("font-size", "11px")
          .style("user-select", "none")
          .style("width", "fit-content")
          .style("height", "100%")
          .style("opacity", "0.7")
          .on("mouseover", (ev) => ev.currentTarget.style.opacity = 1)
          .on("mouseleave", (ev) => ev.currentTarget.style.opacity = 0.7)
          .html((d : flat_node) => {
            if(d.shortname == null) {
              throw new Error("Cannot create label, shortname undefined!");
            } else {
              return(d.shortname);
            }
          })
          .on("click", (ev, d) => {
            extrude_branch({
              node_name: parseInt(d.name),
              params_kept: d.params.slice(Math.ceil(d.params.length / 2))
            })
          });

        ld.append("xhtml:div")
          .style("position", "absolute")
          .style("width", "7px")
          .style("height", "5px")
          .style("bottom", (d : flat_node) => {
            if(d.x_pos == null || d.label_y == null) {
              return 0;
            } else {
              return d.x_pos > d.label_y ? null : 0
            }
          })
          .style("top", (d : flat_node) => {
            if(d.x_pos == null || d.label_y == null) {
              return 0;
            } else {
              return d.x_pos > d.label_y ? null : 0
            }
          })
          .style("left", "-2px")
          .style("background", "black")
          .style("border-radius", "1.5px")
          //.style("transform", "rotate(45deg)")
          
        return g;
      },
    (update) => {
      update.select("circle").transition()
        .attr("cx", (d : flat_node) => x(d.ered))
        .attr("cy", (d: flat_node) => y(d.x_pos ?? 0));
      update.select(".label_fo").transition()
        .attr("x", (d : flat_node) => x(0.003 + d.ered))
        .attr("y", (d: flat_node) => y(d.label_y ?? 0))
      return update
    },
    (exit) => {
      exit.transition().attr("opacity", 0).remove()
    });
  }

  onMount(() => {
    const svg = d3.select("#tree");
    let xaxis = d3.axisBottom(x);
    xaxis(svg.select("#x_axis"));

    const ctx = (new OffscreenCanvas(1000, 1000)).getContext('2d');
    if(ctx == null) {
      throw new Error("Cannot compute width of text, rendering context undefined!");
    }
    ctx.font = "11px sans-serif";

    function compute_width(text: string | undefined) {
      if(text == null) {
        return 0;
      }
      if(ctx == null) {
        throw new Error("Cannot compute width of text, rendering context undefined!");
      } else {
        const text_measure = ctx?.measureText(text);
        return(x.invert(text_measure.actualBoundingBoxLeft + text_measure.actualBoundingBoxRight));
      }
    }

    if(ctx == null) {
      throw new Error("Cannot proceed, rendering context null!");
    } else {
      // const tree = annotate_tree(test_tree_flat, l_height, 1, ctx, x);
      // draw_tree(tree, compute_width);

      ws.handle_message((data) => {
        const tree = annotate_tree(data, l_height, height, ctx, x);
        draw_tree(tree, compute_width);
        console.log("Tree is now:")
        console.log(tree);
      });

      draw_tree([], compute_width);
      console.log("Calling get_tree.")
      get_tree([]);

      document.addEventListener("keydown", (ev) => {
        if(ev.code == "KeyR") reset_tree([]);
      });
    }
  });
</script>

<div id="tree_container">
  <svg id="tree" height={height} width={width + 100}>
    <g id="tree_g" transform="translate(10 0)"></g>
    <g id="x_axis" transform={`translate(10 ${0.96 * height})`}></g>
  </svg>
</div>

<style>
  #tree_container {
    display: flex;
    flex-direction: row;
    justify-content: center;
  }
</style>