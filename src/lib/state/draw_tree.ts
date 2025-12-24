import type { flat_node, flat_tree, flat_branch } from "./types.ts";
import type { SvelteMap } from "svelte/reactivity";
import type { render_config, global_data, coordinates, event_handlers } from "./draw_data.ts";
import { short_name } from "./short_names.ts";
import { global_latex, type name_t } from "./names.ts"
import * as d3 from "d3";

// Draw node details on hover
export function draw_highlight(
  tree : flat_tree,
  x : d3.ScaleLinear<number, number>,
  config : render_config,
  svg_height : number
) {
  d3.select("#highlight_container")
    .selectAll(".highlight_box")
    .data(tree, (d : flat_node) => d.name)
    .join(
      (enter) => {
        const g = enter.append("g")
                        .attr("class", "highlight_box");

        g.append("line")
          .attr("x1", (d) => x(d.ered))
          .attr("y1", -10)
          .attr("x2", (d) => x(d.ered))
          .attr("y2", 5)
          .style("stroke", "#2a57ecff")

        const fo = g.append("foreignObject")
          .attr("x", (d) => {
            const ered_round = Math.round(100 * d.ered) / 100;
            const off = ered_round.toString().length > 1 ? 45/2 : 20/2;
            return(x(d.ered) - off);
          })
          .attr("y", 3)
          .attr("height", "40px")
          .attr("width", (d) => {
            const ered_round = Math.round(100 * d.ered) / 100;
            return(ered_round.toString().length > 1 ? "45px" : "20px");
          });

        fo.append("xhtml:div")
          .attr("width", "")
          .attr("height", (config.label_height - 2) + "px")
          .style("display", "flex")
          .style("flex-direction", "row")
          .style("background", "white")
          .style("border-color", "#2a57ecff")
          .style("align-items", "center")
          .style("justify-content", "center")
          .style("font-family", "sans-serif")
          .style("box-sizing", "border-box")
          .style("border-width", "0.12rem")
          .style("border-style", "solid")
          .style("border-radius", "4px")
          .style("padding", "4px")
          .html((d) => {
            const ered_round = Math.round(1000 * d.ered) / 1000;
            return(ered_round.toString());
          });

        g.transition().duration(config.transition_dur/ 4).attr("opacity", 1);
        return(g);
      },
      (update) => update,
      (exit) => exit.transition().duration(config.transition_dur / 4).attr("opacity", 0).remove()
    );

    d3.select("#vert_line_container").selectAll(".vline")
      .data(tree, (d : flat_node) => d.name)
      .join(
        (enter) => {
          const g = enter.append("g").attr("class", "vline")

          g.append("line")
          .attr("x1", (d) => x(d.ered))
          .attr("y1", 0)
          .attr("x2", (d) => x(d.ered))
          .attr("y2", 1.2 * svg_height)
          .style("stroke", "#2a57ecff")

          g.transition().duration(config.transition_dur / 4).attr("opacity", 1);

          return(g)
        },
        (update) => update,
        (exit) => exit.transition().duration(config.transition_dur / 4).attr("opacity", 0).remove()
      );
}

// Draw tree nodes and branches
export function draw_geometry(
  tree : flat_tree,
  draw_group : string, 
  coord: coordinates,
  config: render_config,
  id_mod : string = "",
  scale = 1,
  handlers: event_handlers
) {

  const tree_elem = d3.select("#" + draw_group + id_mod);

  const link = d3.line<flat_node>(
    (d) => coord.x(d.ered),
    (d) => coord.y(d.x_pos ?? 0)
  ).curve(d3.curveStepBefore);

  const branch_data = tree.filter((n) => n.parent != "").map((n) => {
    const pn = tree.find((n2) => n2.name === n.parent);
    return pn == null ? null : {child: n, parent: pn};
  }).filter((branch) => branch != null);

  // Draw branches
  tree_elem.selectAll(".tree_branch")
    .data(
      branch_data,
      (d: flat_branch) => `${d.parent.name}-->${d.child.name}`
    )
    .join(
      (enter) => {
        const g = enter.append("g").attr("class", "tree_branch");
        if(!config.draw_static) {
          g.transition().duration(config.transition_dur).attr("opacity", 1);
        }

        g.append("path").attr("d", (d) => link([d.parent, d.child]))
          .attr("class", "branch_path")
          .style("stroke", config.highlight_color)
          .style("stroke-width", Math.pow(scale, 4) * 2)
          .style("fill", "none")
          .attr("id", (d) => `branch-${d.child.shortname}-${d.parent.shortname}${id_mod}`);

        const path_select = g.append("path").attr("d", (d) => link([d.parent, d.child]))
          .attr("class", "select_path")
          .style("stroke", "red")
          .style("stroke-width", 15)
          .style("fill", "none")
          .style("opacity", 0)
        
        if(!config.draw_static) {
          path_select.on("mousedown", (_ev, d) => handlers.branch_select(d));
        }

        return g;
      },
      (update) => {
        update.select(".branch_path").transition().duration(config.transition_dur)
                .attr("d", (d) => link([d.parent, d.child]))
        update.select(".select_path").transition().duration(config.transition_dur)
                .attr("d", (d) => link([d.parent, d.child]))
        return update;
      },
      (exit) => {
        exit.transition().duration(config.transition_dur).attr("opacity", 0).remove();
      }
    );

  // Draw nodes
  const tree_nodes = tree_elem.selectAll(".tree_node")
    .data(tree, (d) => (d as flat_node).name);

  tree_nodes.join(
    (enter) => {
      const g = enter.append("g")
        .attr("id", (d) => `${d.name}${id_mod}`)
        .attr("class", "tree_node");

      if(!config.draw_static) {
        g.transition().duration(config.transition_dur).attr("opacity", 1);
      }

      g.append("rect")
        .attr("id", (d) => `${d.name}_rect${id_mod}`)
        .attr("class", "node_rect")
        .attr("width", scale * 7)
        .attr("height", scale * 7)
        .attr("x", (d : flat_node) => coord.x(d.ered) - (scale * 7) / 2)
        .attr("y", (d: flat_node) => coord.y(d.x_pos ?? 0) - (scale * 7) / 2)
        .attr("rx", scale * 1.5)
        .style("fill", config.draw_color)
        .style("stroke", config.highlight_color)
        .style("stroke-width", scale * 3.5)
      
      return(g);
    },
    (update) => {
      update.select(".node_rect").transition().duration(config.transition_dur)
        .attr("x", (d : flat_node) => coord.x(d.ered) - 4)
        .attr("y", (d: flat_node) => coord.y(d.x_pos ?? 0) - 4);
      return update
    },
    (exit) => {
      exit.transition().duration(config.transition_dur).attr("opacity", 0).remove();
    }
  );
}

// Draw complete tree with labels
export function draw_tree(
  tree : flat_tree, 
  coord: coordinates,
  global: global_data,
  config: render_config,
  handlers: event_handlers,
  names: SvelteMap<string, name_t>) {

  // Setup data
  const id_mod = config.draw_static ? "_static" : "";
  console.warn([...tree.map((fn) => fn.label_y)])
  const max_y = coord.y(tree.map((fn) => fn.label_y).reduce((p,n) => Math.max(p,n))) + (2 * config.label_height);

  // Define pan behavior
  // const pan = d3.zoom();

  // if(!config.draw_static) {
  //   pan.on('zoom', (e) => {
  //     if(e?.sourceEvent?.wheelDeltaY) {
  //       const dy = -1 * e.sourceEvent.wheelDeltaY;
  //       const t = e.transform;
  //       t.k = 1;
  //       t.x = 0;
  //       cur_y = Math.min(Math.max(0, cur_y + dy), max_y);
  //       t.y = -1 * cur_y;
  //       d3.select("#tree_layers" + id_mod).attr("transform", t.toString());
  //       d3.select("#label_layer").attr("transform", t.toString());
  //     } 
  //   });

  //   d3.select("#tree").call(pan);
  // }

  // Draw shaded rectangle representing minimum var reduction from globals alone
  const limit_data = (global.limit != undefined && config.show_globals) ? [global.limit] : [];

  d3.select("#tree_outer").selectAll(".global_limit_rect")
    .data(limit_data)
    .join(
      (enter) => {
        const grect = enter.append("rect")
          .attr("class", "global_limit_rect")
          .attr("x", (d) => coord.x(d))
          .attr("y", -10)
          .style("fill", "black")
          .style("opacity", "10%")
          .attr("height", "110%")
          .attr("width", "100%");

        return grect;
      },
      (update) => update,
      (exit) => exit.remove()
    );

  // Draw label for global limit cutoff
  d3.select("#top_bar" + id_mod).selectAll(".label_fo_global")
    .data(limit_data)
    .join(
      (enter) => {
        const fo = enter.append("foreignObject")
          .attr("class", "label_fo_global")
          .attr("x", (d) => coord.x(d))
          .attr("y", 0)
          .attr("height", () => config.label_height + "px") //y(l_height)
          .attr("width", "1000px")

        const ldd = fo.append("xhtml:div")
          .style("height", "100%")
          .style("padding", "1px");

        const ld = ldd.append("xhtml:div")
          .attr("id", "global_limit_label")
          .attr("class", "label-div")
          .style("display", "flex")
          .style("flex-direction", "row")
          .style("background", "white")
          .style("border-color", "black")
          .style("align-items", "center")
          .style("font-family", "sans-serif")
          .style("box-sizing", "border-box")
          .style("border-width", "0.12rem")
          .style("border-style", "solid")
          .style("border-radius", "8px")
          .style("border-top-left-radius", "8px")
          .style("border-bottom-left-radius", "2px")
          .style("font-size", "12px")
          .style("font-weight", "bold")
          .style("width", "fit-content")
          .style("height", (config.label_height - 2) + "px")

        ld.append("xhtml:div")
            .attr("class", "global_tag")
            .style("height", "100%")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("align-items", "center")
            .style("padding-left", "6px")
            .style("padding-right", "6px")
            .style("font-weight", "normal")
            .style("user-select", "none")
            .style("border-right", "1px solid #444444")
            .style("font-weight", "bold")
            .html(global_latex);

        const plist = ld.append("xhtml:div")
            .attr("class", "global_params_list")
            .style("height", "100%")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("gap", "6px")
            .style("align-items", "center")
            .style("padding-left", "6px")
            .style("padding-right", "6px")
            .style("font-weight", "normal")
            .style("user-select", "none")
            .style("border-color", "blue")

        plist.selectAll(".global_name").data(short_name(global.params, names), (d : string) => d)
            .join(
              (enter) => {
                const pdiv = enter.append("div")
                  .attr("class", "global_name")
                  .style("border-radius", "3px")
                  .style("background", "#ffffffff")
                  .style("border", "1px solid #3b3b3bff")
                  .style("display", "flex")
                  .style("flex-direction", "row")
                  
                pdiv.append("div")
                  .style("font-weight", "bold")
                  .style("padding", "5px")
                  .style("height", "100%")
                  .html((d : string) => d.split("[")[0])

                pdiv.append("div")
                  .style("padding", (d : string) => d.split("[").length > 1 ? "5px" : "0px")
                  .style("border-left", "1px solid #3b3b3bff")
                  .style("visibility", (d : string) => d.split("[").length > 1 ? "visible" : "hidden")
                  .html((d : string) => {
                    const end_str = d.split("[");
                    if(end_str.length > 1) {
                      return(end_str[1].substring(0, end_str[1].length - 1));
                    } else {
                      return("");
                    }
                  })

                return(pdiv)
              },
            (update) => update,
            (exit) => exit.remove()
          );
        return fo;
      },
      (update) => update,
      (exit) => exit.remove()
    );

  // Draw tree nodes and branches
  draw_geometry(
    tree, "tree_g", coord, config, id_mod, 1, handlers
  );

  // Draw labels
  const tree_nodes = d3.select("#label_layer").selectAll(".tree_node_label")
                              .data(tree, (d) => (d as flat_node).name);
  // Define node labels
  const label_selection = tree_nodes.join((enter) => {

      const g = enter.append("g")
                    .attr("id", (d) => {
                      console.log(`New node label for ${d.name}`);
                      return(`${d.name}${id_mod}`)
                    })
                    .attr("class", "tree_node_label");

      if(!config.draw_static) {
        g.transition().duration(config.transition_dur).attr("opacity", 1);
      }

      const fo = g.append("foreignObject")
        .attr("class", "label_fo")
        .attr("x", (d : flat_node) => coord.x(0.003 + d.ered))
        .attr("y", (d: flat_node) => coord.y(d.label_y ?? 0))
        .attr("height", () => config.label_height + "px") //y(l_height)
        .attr("width", "1000px");

      const ldd = fo.append("xhtml:div")
                    .style("height", "100%")
                    .style("padding", "1px");

      const ld = ldd.append("xhtml:div")
        .attr("id", (d) => `${d.name}_div${id_mod}`)
        .attr("class", "label-div")
        .style("display", "flex")
        .style("flex-direction", "row")
        .style("background", "#fcfcfcff")
        .style("border-color", "black")
        .style("align-items", "center")
        .style("font-family", "sans-serif")
        .style("box-sizing", "border-box")
        .style("border-width", "0.12rem")
        .style("border-style", "solid")
        .style("border-radius", "8px")
        .style("border-top-left-radius", (d) => d.x_pos > d.label_y ? "8px" : "2px")
        .style("border-bottom-left-radius", (d) => d.x_pos > d.label_y ? "2px" : "8px")
        .style("font-size", (1.2 * config.label_height * 14 / 36) + "px")
        .style("font-weight", "bold")
        .style("width", "fit-content")
        .style("height", (config.label_height - 2) + "px")
        
      if(!config.draw_static) {
        fo.on("mousedown", (_ev, d) => handlers.node_select(d));

        ld.on("mouseover", (_ev, d) => handlers.node_hover(d));

        ld.on("mouseleave", (_ev, d) => handlers.node_unhover(d));
      }

      ld.append("xhtml:div")
        .attr("class", "depth_tag")
        .style("height", "100%")
        .style("display", "flex")
        .style("flex-direction", "row")
        .style("align-items", "center")
        .style("padding-left", "6px")
        .style("padding-right", "0px")
        .style("font-weight", "normal")
        .style("font-size", "0.7em")
        .style("user-select", "none")
        .html((d : flat_node) => d.depth.toString());

      ld.append("xhtml:div")
        .attr("class", "params_list")
        .style("height", "100%")
        .style("display", "flex")
        .style("flex-direction", "row")
        .style("gap", "6px")
        .style("align-items", "center")
        .style("padding-left", "6px")
        .style("padding-right", "6px")
        .style("font-weight", "normal")
        .style("user-select", "none")
        .style("border-color", "blue")
      
      return g;
  },
  (update) => {
    update.select(".label_fo").transition().duration(config.transition_dur)
      .attr("x", (d : flat_node) => coord.x(0.003 + d.ered))
      .attr("y", (d: flat_node) => coord.y(d.label_y ?? 0));
    update.select(".label-div")
      .style("border-top-left-radius", (d) => d.x_pos > d.label_y ? "8px" : "2px")
      .style("border-bottom-left-radius", (d) => d.x_pos > d.label_y ? "2px" : "8px");
    update.select(".depth_tag")
      .html((d : flat_node) => {
        console.log(`Updating node label for ${d.name}`);
        return(d.depth.toString())
    });
    return update
  },
  (exit) => {
    exit.transition().duration(config.transition_dur).attr("opacity", 0).remove();
  });

  // Fill in node label content
  const params_lists = label_selection.select(".params_list");
  if(params_lists.size() > 0) {
    params_lists.selectAll(".param_name")
    .data((d : flat_node) => {
          return(d.param_names);
        }, (d : flat_node) => d.param_names?.join(",") ?? "none")
        .join((enter) => {
          const pdiv = enter.append("div")
            .attr("class", "param_name")
            .style("border-radius", "3px")
            .style("background", "#ffffffff")
            .style("border", "1px solid #3b3b3bff")
            .style("height", "70%")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("overflow-y", "hidden")
            
          pdiv.append("div")
            .attr("class", "pname_div")
            .style("font-weight", "bold")
            .style("padding-left", "8px")
            .style("padding-right", "8px")
            .style("height", "100%")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("align-items", "center")
            .html((d : string) => d.split("[")[0])

          pdiv.append("div")
            .attr("class", "pindex_div")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("align-items", "center")
            .style("padding-left", (d : string) => d.split("[").length > 1 ? "8px" : "0px")
            .style("padding-right", (d : string) => d.split("[").length > 1 ? "8px" : "0px")
            .style("border-left", "1px solid #979797ff")
            .style("visibility", (d : string) => d.split("[").length > 1 ? "visible" : "hidden")
            .style("font-size", "0.7em")
            .style("font-family", "Verdana")
            .html((d : string) => {
              const end_str = d.split("[");
              if(end_str.length > 1) {
                return(end_str[1].substring(0, end_str[1].length - 1));
              } else {
                return("");
              }
            })
          return(pdiv)
        },
      (update) => {
        update.select(".pname_div")
          .html((d : string) => {
            return(d.split("[")[0])
          })
        return(update)
      },
      (exit) => {
        exit.remove()
      }
    );
  } else {
    console.error("No parameter lists found during tree construction!")
  }

  return(max_y)
}
