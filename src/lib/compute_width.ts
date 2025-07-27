let ctx : OffscreenCanvasRenderingContext2D | undefined = undefined;

export function setup_context(cvs : OffscreenCanvas) {
  ctx = cvs.getContext('2d');
  if(ctx == null) {
    throw new Error("Cannot compute width of text, rendering context undefined!");
  }
  ctx.font = "11px sans-serif";
}

export function compute_width(text: string | undefined, x: d3.ScaleLinear<number, number>) {
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