import katex from 'katex'

export type name_t = {
  type: "short" | "latex",
  name: string,
  formatted_name: string
}

export const global_latex = katex.renderToString("\\bar{g}", {output: "html"});

export function make_short(name: string) : name_t {
  const sname =  name.split("_").map((s) => s[0]).join("");
  return({
    type: "short",
    name: sname,
    formatted_name: katex.renderToString(
      "\\mathrm{" + sname + "}",
      { output: 'html' }
    )
  })
}