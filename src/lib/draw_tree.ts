  import * as d3 from "d3";
  import { type flat_node, type flat_tree, type flat_branch } from "./tree.ts";
  import { type label_data_t, selection_channels } from "./selection.svelte.ts";
  
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
  
  export function draw_tree(
    tree : flat_tree, 
    x : d3.ScaleLinear<number, number>,
    y : d3.ScaleLinear<number, number>,
    l_height : number,
    label_handler : (d : flat_node) => label_data_t,
    branch_handler : (d : flat_branch) => object,
    ss : CSSStyleSheet) {

    const tree_elem = d3.select("#tree_g");

    const max_y = y(tree.map((fn) => fn.x_pos).reduce((p,n) => Math.max(p,n)) - 1) + 45;
    const pan = d3.zoom();

    pan.on('zoom', (e) => {
      if(e?.sourceEvent?.wheelDeltaY) {
        const dy = -1 * e.sourceEvent.wheelDeltaY;
        const t = e.transform;
        t.k = 1;
        t.x = 10;
        cur_y = Math.min(Math.max(0, cur_y + dy), max_y);
        t.y = -1 * cur_y;
        d3.select("#tree_g").attr("transform", t.toString());
      } 
    });

    d3.select("#tree").call(pan);

    const branch_data = tree.filter((n) => n.parent != "").map((n) => {
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

    const link = d3.line<flat_node>(
      (d) => x(d.ered),
      (d) => y(d.x_pos ?? 0)
    ).curve(d3.curveStepBefore);

    // const tree_layer = tree_elem.append('g');
    //const select_layer = tree_elem.append('g');

    tree_elem.selectAll(".tree_branch")
             .data(branch_data, (d) => `${(d as flat_branch).parent.name}-->${(d as flat_branch).child.name}`)
             .join((enter) => {
                const g = enter.append("g").attr("class", "tree_branch");
                g.transition().attr("opacity", 1);

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                     .attr("class", "branch_path")
                     .style("stroke", "#444444")
                     .style("stroke-width", 2)
                     .style("fill", "none")
                     .attr("id", (d) => `branch-${d.child.shortname}-${d.parent.shortname}`);

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                         .attr("class", "select_path")
                         .style("stroke", "red")
                         .style("stroke-width", 15)
                         .style("fill", "none")
                         .style("opacity", 0)
                         .on("mouseover", (_ev, d) => {
                           const vis_path = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`);
                           vis_path?.setAttribute("stroke", "#0161df");
                         })
                         .on("mouseleave", (_ev, d) => {
                           const vis_path = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`);
                           vis_path?.setAttribute("stroke", "black");
                         })
                         .on("click", (_ev, d) => {
                            const target_classes = document.getElementById(`branch-${d.child.shortname}-${d.parent.shortname}`).classList;
                            const cur_selected = target_classes.contains("branch_selected")
                            const branch_styles = branch_handler(d);
                            if(Object.keys(branch_styles).length === 0) {
                              return;
                            }
                            if(branch_rule_index != null) {
                              ss.deleteRule(branch_rule_index);
                            }
                            branch_rule_index = ss.insertRule(to_style_string(".branch_selected", branch_styles));
                            const branches = document.getElementsByClassName("branch_path");
                            for(let bi = 0; bi < branches.length; ++bi) {
                              branches[bi].classList.remove("branch_selected");
                            }
                            if(!cur_selected) {
                              target_classes.add("branch_selected");
                            }
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
              exit.transition().attr("opacity", 0).remove();
            }
          );

    const tree_nodes = tree_elem.selectAll(".tree_node").data(tree, (d) => (d as flat_node).name);
    tree_nodes.join((enter) => {
        const g = enter.append("g")
                     .attr("id", (d) => `${d.name}`)
                     .attr("class", "tree_node");

        g.transition().attr("opacity", 1);

        g.append("rect")
          .attr("id", (d) => `${d.name}_rect`)
          .attr("class", "node_rect")
          .attr("width", 9)
          .attr("height", 9)
          .attr("x", (d : flat_node) => x(d.ered) - 4.5)
          .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - 4.5)
          .attr("rx", "1.5")
          .style("fill", "#444444")

        const fo = g.append("foreignObject")
          .attr("class", "label_fo")
          .attr("x", (d : flat_node) => x(0.003 + d.ered))
          .attr("y", (d: flat_node) => y(d.label_y ?? 0))
          .attr("height", () => l_height + "px") //y(l_height)
          .attr("width", "1000px"); //(d: flat_node) =>  x(10 * compute_width(d.param_names, x))

        const ld = fo.append("xhtml:div")
          .attr("id", (d) => `${d.name}_div`)
          .attr("class", "label-div")
          .style("display", "flex")
          .style("flex-direction", "row")
          // .style("background", "#edededff")
          .style("background", "white")
          .style("align-items", "center")
          .style("font-family", "sans-serif")
          .style("box-sizing", "border-box")
          .style("border-width", "0.1rem")
          .style("border-color", "black")
          .style("border-style", "solid")
          // .style("border-radius", "0.18rem")
          .style("border-radius", "8px")
          .style("border-top-left-radius", (d) => d.x_pos > d.label_y ? "8px" : "0px")
          .style("border-bottom-left-radius", (d) => d.x_pos > d.label_y ? "0px" : "8px")
          .style("font-size", "12px")
          .style("font-weight", "bold")
          .style("user-select", "none")
          .style("width", "fit-content")
          .style("height", "100%")
          .on("mouseover", (ev) => {
            ev.currentTarget.style.opacity = 1;
          })
          .on("click", (_ev, d) => {
            const label_data = label_handler(d);
            if(label_data == null) {
              for(const channel of selection_channels) {
                const cn = `${channel}_label_selected`;
                const els = Array.from(document.getElementsByClassName(cn));
                for(let ei = 0; ei < els.length; ++ei) {
                  els[ei].classList.remove(cn);
                }
                const cnr = `${channel}_rect_selected`;
                const elsr = Array.from(document.getElementsByClassName(cnr));
                for(let ei = 0; ei < elsr.length; ++ei) {
                  elsr[ei].classList.remove(cnr);
                }
              }
            } else {
              const cn = `${label_data.channel}_label_selected`;
              const cnr = `${label_data.channel}_rect_selected`;
              const els = Array.from(document.getElementsByClassName(cn));
              for(let ei = 0; ei < els.length; ++ei) {
                els[ei].classList.remove(cn);
              }
              const elsr = Array.from(document.getElementsByClassName(cnr));
              for(let ei = 0; ei < elsr.length; ++ei) {
                elsr[ei].classList.remove(cnr);
              }
              for(let ni = 0; ni < label_data.target.length; ++ni) {
                document.getElementById(`${label_data.target[ni]}_div`).classList.add(cn);
                document.getElementById(`${label_data.target[ni]}_rect`).classList.add(cnr);
              }
            }
          });

          ld.append("xhtml:div")
            .attr("class", "depth_tag")
            .style("height", "100%")
            .style("display", "flex")
            .style("flex-direction", "row")
            .style("align-items", "center")
            .style("padding-left", "6px")
            .style("padding-right", "0px")
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
          
          return g;
    },
    (update) => {
      update.select(".node_rect").transition()
        .attr("x", (d : flat_node) => x(d.ered) - 4)
        .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - 4);
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
              .style("background", "#white")
              .style("border-radius", "3px")
              .style("background", "white")
              .style("border", "1px solid #b0b0b0ff")
              .style("display", "flex")
              .style("flex-direction", "row")
              
            pdiv.append("div")
              .style("font-weight", "bold")
              .style("padding", "5px")
              .style("height", "100%")
              .html((d : string) => d.split("[")[0])

            pdiv.append("div")
              .style("padding", (d : string) => d.split("[").length > 1 ? "5px" : "0px")
              .style("border-left", "1px solid #c6c6c6ff")
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