  import * as d3 from "d3";
  import { type flat_node, type flat_tree, type flat_branch } from "./tree.ts";
  import { compute_width } from "./compute_width.ts";

  type label_data_t = {
    selections: string,
    action: string
  }

  // type label_rule_index_t = {
  //   main: undefined | number,
  //   alt: undefined | number
  // }

  // let label_rule_index : label_rule_index_t = {
  //   main: undefined,
  //   alt: undefined
  // }
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
    }
  }
  
  export function draw_tree(
    tree : flat_tree, 
    x : d3.ScaleLinear<number, number>,
    y : d3.ScaleLinear<number, number>,
    l_height : number,
    label_handler : (d : flat_node) => label_data_t,
    branch_handler : (d : flat_branch) => object,
    ss : CSSStyleSheet) {
    const tree_elem = d3.select("#tree_g");

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

    tree_elem.selectAll(".tree_branch")
             .data(branch_data, (d) => `${(d as flat_branch).parent.name}-->${(d as flat_branch).child.name}`)
             .join((enter) => {
                const g = enter.append("g").attr("class", "tree_branch");
                g.transition().attr("opacity", 1);

                g.append("path").attr("d", (d) => link([d.parent, d.child]))
                     .attr("class", "branch_path")
                     .style("stroke", "black")
                     .style("stroke-width", 2.5)
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
              exit.transition().attr("opacity", 0).remove()
            }
          );

    tree_elem.selectAll(".tree_node")
      .data(tree, (d) => (d as flat_node).name)
      .join((enter) => {
        const g = enter.append("g")
                     .attr("id", (d) => `${d.name}`)
                     .attr("class", "tree_node");

        g.transition().attr("opacity", 1);

        // g.append("circle")
        //   .attr("r", 5)
        //   .attr("cx", (d : flat_node) => x(d.ered))
        //   .attr("cy", (d: flat_node) => y(d.x_pos ?? 0))
        //   .style("fill", "black")
        //   //.attr("stroke", "white")
        //   .style("stroke-width", 2.5);

        g.append("rect")
          .attr("class", "node_rect")
          .attr("width", 9)
          .attr("height", 9)
          .attr("x", (d : flat_node) => x(d.ered) - 4.5)
          .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - 4.5)
          .attr("rx", "1.5")
          .style("fill", "black")
          //.attr("stroke", "white")
          //.style("stroke-width", 2.5);

        const fo = g.append("foreignObject")
          .attr("class", "label_fo")
          .attr("x", (d : flat_node) => x(0.003 + d.ered))
          .attr("y", (d: flat_node) => y(d.label_y ?? 0))
          .attr("height", () => y(l_height))
          .attr("width", (d: flat_node) =>  x(2 * compute_width(d.shortname, x)));

        const ld = fo.append("xhtml:div")
          .attr("class", "label-div")
          .style("background", "white")
          .style("font-family", "sans-serif")
          .style("box-sizing", "border-box")
          .style("padding", "4px")
          .style("border-width", "0.1rem")
          .style("border-color", "black")
          .style("border-style", "solid")
          .style("border-radius", "0.18rem")
          .style("font-size", "11px")
          .style("user-select", "none")
          .style("width", "fit-content")
          .style("height", "100%")
          .style("opacity", "0.7")
          .on("mouseover", (ev) => {
            ev.currentTarget.style.opacity = 1;
          })
          .on("mouseleave", (ev) => ev.currentTarget.style.opacity = 0.7)
          .html((d : flat_node) => {
            if(d.shortname == null) {
              throw new Error("Cannot create label, shortname undefined!");
            } else {
              return(d.shortname);
            }
          })
          .on("click", (ev, d) => {
            const target_classes = ev.currentTarget.classList;
            const label_data = label_handler(d);
            if (label_data.action === 'select') {
              if(label_data.selections != 'main' && label_data.selections != 'alt') {
                console.error('Invalid selection: ' + label_data.selections);
                return;
              }
              const selected = document.getElementsByClassName(`${label_data.selections}_label_selected`);
              for(let mi = 0; mi < selected.length; ++mi) {
                selected[mi].classList.remove(`${label_data.selections}_label_selected`);
              }
              target_classes.add(`${label_data.selections}_label_selected`);
            } else if (label_data.action === 'deselect') {
              if(label_data.selections === 'main' || label_data.selections === 'both') {
                const selected_main = document.getElementsByClassName("main_label_selected");
                for(let mi = 0; mi < selected_main.length; ++mi) {
                  selected_main[mi].classList.remove('main_label_selected');
                }
              }
              if(label_data.selections === 'alt' || label_data.selections === 'both') {
                const selected_alt = document.getElementsByClassName("alt_label_selected");
                for(let ai = 0; ai < selected_alt.length; ++ai) {
                  selected_alt[ai].classList.remove('alt_label_selected');
                }
              }
            }
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
      // update.select("circle").transition()
      //   .attr("cx", (d : flat_node) => x(d.ered))
      //   .attr("cy", (d: flat_node) => y(d.x_pos ?? 0));
      update.select(".node_rect").transition()
        .attr("x", (d : flat_node) => x(d.ered) - 4)
        .attr("y", (d: flat_node) => y(d.x_pos ?? 0) - 4);
      update.select(".label_fo").transition()
        .attr("x", (d : flat_node) => x(0.003 + d.ered))
        .attr("y", (d: flat_node) => y(d.label_y ?? 0))
      return update
    },
    (exit) => {
      exit.transition().attr("opacity", 0).remove()
    });
  }