import { type flat_node, type flat_tree, type flat_branch } from "./tree.ts";
import { selection_channels, selection } from "./selection.svelte.ts";
import * as d3 from "d3";
import { normalizeUrl } from "@sveltejs/kit";

let branch_rule_index : number | undefined = undefined;

  function to_style_string(selector : string, styles : object) {
    let sstr = `${selector} { `;
    for(const [style_name, style_value] of Object.entries(styles)) {
      sstr += `${style_name}: ${style_value} !important; `;
    }
    sstr += '}';
    return sstr;
  }

  export function reset_styles() {
    const branches = document.getElementsByClassName("branch_path");
    for(let bi = 0; bi < branches.length; ++bi) {
      branches[bi].classList.remove("branch_selected");
    }
    const labels = document.getElementsByClassName("label-div");
    for(let li = 0; li < labels.length; ++li) {
      labels[li].classList.remove("main_label_selected");
      labels[li].classList.remove("alt_label_selected");
      labels[li].classList.remove("del_label_selected");
    }
    const rects = document.getElementsByClassName("node_rect");
    for(let ri = 0; ri < rects.length; ++ri) {
      rects[ri].classList.remove("main_rect_selected");
      rects[ri].classList.remove("alt_rect_selected");
      rects[ri].classList.remove("del_rect_selected");
    }
  }

  let cur_y = 0;
  
  export function draw_geometry(
    tree : flat_tree,
    draw_group : string, 
    x : d3.ScaleLinear<number, number>,
    y : d3.ScaleLinear<number, number>,
    branch_handler : (d : flat_branch) => void,
    ss : CSSStyleSheet,
    draw_color : string = "black",
    highlight_color : string = "black",
    draw_static : boolean = false,
    id_mod : string = "",
    scale = 1
  ) {

    const tree_elem = d3.select("#" + draw_group + id_mod);
    if(tree_elem.size() != 1) {
      console.error("Cannot draw geometry! Selected group does not exist.")
    }

    const link = d3.line<flat_node>(
      (d) => x(d.ered),
      (d) => y(d.x_pos ?? 0)
    ).curve(d3.curveStepBefore);

    const branch_data = tree.filter((n) => n.parent != "").map((n) => {
      const pn = tree.find((n2) => n2.name === n.parent);
      if(pn != null) {
        return {
          child: n,
          parent: pn
        }
      } else {
        return null;
      }
    }).filter((branch) => branch != null);

    // Draw branches
    tree_elem.selectAll(".tree_branch")
             .data(branch_data, (d) => `${(d as flat_branch).parent.name}-->${(d as flat_branch).child.name}`)
             .join((enter) => {
                const g = enter.append("g").attr("class", "tree_branch");
                if(!draw_static) {
                  g.transition().attr("opacity", 1);
                }

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                     .attr("class", "branch_path")
                     .style("stroke", highlight_color)
                     .style("stroke-width", Math.pow(scale, 2) * 2)
                     .style("fill", "none")
                     .attr("id", (d) => `branch-${d.child.shortname}-${d.parent.shortname}${id_mod}`);

                const path_select = g.append("path").attr("d", (d) => link([d.parent, d.child]))
                         .attr("class", "select_path")
                         .style("stroke", "red")
                         .style("stroke-width", 15)
                         .style("fill", "none")
                         .style("opacity", 0)
                
                if(!draw_static) {
                  path_select.on("click", (_ev, d) => {
                    branch_handler(d);
                  });
                  //path_select
                  // .on("click", (_ev, d) => {
                  //   const target_classes = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`).classList;
                  //   const cur_selected = target_classes.contains("branch_selected")
                  //   const branch_styles = branch_handler(d);
                  //   if(branch_rule_index != null) {
                  //     ss.deleteRule(branch_rule_index);
                  //   }
                  //   //branch_rule_index = ss.insertRule(to_style_string(".branch_selected", branch_styles));
                  //   const branches = document.getElementsByClassName("branch_path");
                  //   for(let bi = 0; bi < branches.length; ++bi) {
                  //     branches[bi].classList.remove("branch_selected");
                  //   }
                  //   if(!cur_selected) {
                  //     target_classes.add("branch_selected");
                  //   }
                  // });
                }

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
              exit.transition().attr("opacity", 0).remove();
            }
          );

    // Draw nodes
    const tree_nodes = tree_elem.selectAll(".tree_node")
                                .data(tree, (d) => (d as flat_node).name);

    tree_nodes.join((enter) => {
        const g = enter.append("g")
                     .attr("id", (d) => `${d.name}${id_mod}`)
                     .attr("class", "tree_node");

        if(!draw_static) {
          g.transition().attr("opacity", 1);
        }

        g.append("rect")
          .attr("id", (d) => `${d.name}_rect${id_mod}`)
          .attr("class", "node_rect")
          .attr("width", scale * 7)
          .attr("height", scale * 7)
          .attr("x", (d : flat_node) => x(d.ered) - (scale * 7) / 2)
          .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - (scale * 7) / 2)
          .attr("rx", scale * 1.5)
          .style("fill", draw_color)
          .style("stroke", highlight_color)
          .style("stroke-width", scale * 3.5)
        
        return(g);
    },(update) => {
      update.select(".node_rect").transition()
        .attr("x", (d : flat_node) => x(d.ered) - 4)
        .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - 4);
      return update
    },
    (exit) => {
      exit.transition().attr("opacity", 0).remove();
    });
  }

  export function draw_tree(
    tree : flat_tree, 
    x : d3.ScaleLinear<number, number>,
    y : d3.ScaleLinear<number, number>,
    l_height : number,
    label_handler : (d : flat_node) => void,
    branch_handler : (d : flat_branch) => void,
    ss : CSSStyleSheet,
    draw_color : string = "black",
    draw_static : boolean = false) {

    if(!draw_static){   
      // const sel_colors = {
      //   "main" : "#4040ea",
      //   "alt" : "#c20ca4ff",
      //   "del" : "#f19513ff"
      // }
      const sel_colors = {
        "main" : "#2a57ecff",
        "alt" : "#910a8a",
        "del" : "#d48e1d"
      }

      selection?.on_change(() => {
        for(const channel of selection_channels) {
          const ft = tree.filter((node) => selection.has(node.name, channel));
          if(ft.length != 0) {
            console.warn(channel + " is nonempty!");
          } else {
            console.warn(channel + " is empty!");
          }
          draw_geometry(
            ft, "tree_g", x, y,
            () => {}, document.styleSheets[0],
            sel_colors[channel], sel_colors[channel], true, "_" + channel, 1.15
          );
        }
        // Clear all selection classes
        for(const channel of selection_channels) {
          const cn = `${channel}_label_selected`;
          const els = Array.from(document.getElementsByClassName(cn));
          for(let ei = 0; ei < els.length; ++ei) {
            els[ei].classList.remove(cn);
          }
        }
        // Add selection classes for newly selection items
        for(const channel of selection_channels) {
          const cn = `${channel}_label_selected`;
          const sel_nodes = selection.nodes(channel);
          for(let ni = 0; ni < selection.size(channel); ++ni) {
            document.getElementById(`${sel_nodes[ni]}_div`).classList.add(cn);
          }
        }
      });
    }

    // Setup data

    const id_mod = draw_static ? "_static" : "";
    const tree_elem = d3.select("#tree_g" + id_mod);
    const max_y = y(tree.map((fn) => fn.x_pos).reduce((p,n) => Math.max(p,n)) - 1) + 45;

    // Define pan behavior
    const pan = d3.zoom();

    if(!draw_static) {
      pan.on('zoom', (e) => {
        if(e?.sourceEvent?.wheelDeltaY) {
          const dy = -1 * e.sourceEvent.wheelDeltaY;
          const t = e.transform;
          t.k = 1;
          t.x = 10;
          cur_y = Math.min(Math.max(0, cur_y + dy), max_y);
          t.y = -1 * cur_y;
          d3.select("#tree_outer" + id_mod).attr("transform", t.toString());
        } 
      });

      d3.select("#tree").call(pan);
    }

    // Draw tree nodes and branches
    draw_geometry(
      tree, "tree_g", x, y, branch_handler, ss, draw_color, draw_color, draw_static, id_mod
    );

    // Draw labels
    const tree_nodes = tree_elem.selectAll(".tree_node_label")
                                .data(tree, (d) => (d as flat_node).name);
    // Define node labels
    tree_nodes.join((enter) => {

        const g = enter.append("g")
                     .attr("id", (d) => `${d.name}${id_mod}`)
                     .attr("class", "tree_node_label");

        if(!draw_static) {
          g.transition().attr("opacity", 1);
        }

        const fo = g.append("foreignObject")
          .attr("class", "label_fo")
          .attr("x", (d : flat_node) => x(0.003 + d.ered))
          .attr("y", (d: flat_node) => y(d.label_y ?? 0))
          .attr("height", () => l_height + "px") //y(l_height)
          .attr("width", "1000px"); //(d: flat_node) =>  x(10 * compute_width(d.param_names, x))

        const ldd = fo.append("xhtml:div")
                      .style("height", "100%")
                      .style("padding", "1px");

        const ld = ldd.append("xhtml:div")
          .attr("id", (d) => `${d.name}_div${id_mod}`)
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
          .style("border-top-left-radius", (d) => d.x_pos > d.label_y ? "8px" : "0px")
          .style("border-bottom-left-radius", (d) => d.x_pos > d.label_y ? "0px" : "8px")
          .style("font-size", "12px")
          .style("font-weight", "bold")
          // .style("user-select", "none")
          .style("width", "fit-content")
          .style("height", (l_height - 2) + "px")
          
        if(!draw_static) {
          fo.on("mousedown", (_ev, d) => {
            console.log("IM CLICKED")
            label_handler(d);
          });
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
          .style("user-select", "none")
          // .style("border-right", "1px solid #444444")
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
        
        return g;
    },
    (update) => {
      update.select(".label_fo").transition()
        .attr("x", (d : flat_node) => x(0.003 + d.ered))
        .attr("y", (d: flat_node) => y(d.label_y ?? 0));
      update.select(".label-div")
        .style("border-top-left-radius", (d) => d.x_pos > d.label_y ? "8px" : "0px")
        .style("border-bottom-left-radius", (d) => d.x_pos > d.label_y ? "0px" : "8px");
      update.select(".depth_tag")
        .html((d : flat_node) => d.depth.toString());
      return update
    },
    (exit) => {
      exit.transition().attr("opacity", 0).remove();
    });

    // Fill in node label content
    const params_lists = d3.selectAll(".params_list");
    if(params_lists.size() > 0) {
      const param_names = params_lists.selectAll(".param_name"); //<d3.BaseType, HTMLElement>
      console.log(param_names.nodes());
      param_names.data((d : flat_node) => {
            return(d.param_names);
          })
          .join((enter) => {
            const pdiv = enter.append("div")
              .attr("class", "param_name")
              .style("border-radius", "3px")
              .style("background", "#ffffffff")
              .style("border", "1px solid #979797ff")
              .style("display", "flex")
              .style("flex-direction", "row")
              
            pdiv.append("div")
              .style("font-weight", "bold")
              .style("padding", "5px")
              .style("height", "100%")
              .html((d : string) => d.split("[")[0])

            pdiv.append("div")
              .style("padding", (d : string) => d.split("[").length > 1 ? "5px" : "0px")
              .style("border-left", "1px solid #979797ff")
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
    }
  }
