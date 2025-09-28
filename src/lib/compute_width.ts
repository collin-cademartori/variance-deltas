let ctx : OffscreenCanvasRenderingContext2D | undefined = undefined;

export function setup_context(cvs : OffscreenCanvas) {
  ctx = cvs.getContext('2d');
  if(ctx == null) {
    throw new Error("Cannot compute width of text, rendering context undefined!");
  }
  ctx.font = "11px sans-serif";
}

export function compute_width(text: string[] | undefined, x: d3.ScaleLinear<number, number>) {
  if(text == null) {
    return 0;
  }
  if(ctx == null) {
    throw new Error("Cannot compute width of text, rendering context undefined!");
  } else {
    //console.log(text)
    const text_measures = text.map((txt) => {
      const tm = ctx?.measureText(txt);
      return(tm.actualBoundingBoxLeft + tm.actualBoundingBoxRight + 25)
    });
    return(x.invert(20 + text_measures.reduce((p,v) => p+v, 0)));
  }
}