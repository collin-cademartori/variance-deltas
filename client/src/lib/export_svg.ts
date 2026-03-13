export function export_svg(svg : SVGElement, cvs : OffscreenCanvas, anchor : HTMLAnchorElement) {
  const svg_xml = (new XMLSerializer()).serializeToString(svg);
  const uri = `data:image/svg+xml;charset=utf-8,${encodeURIComponent(svg_xml)}`;
  const img = new Image(); 
  img.src = uri;
  img.onload = () => {
    cvs.width = 4 * img.naturalWidth;
    cvs.height = 4 * img.naturalHeight;
    const ctx = cvs.getContext('2d');
    if(ctx == null) {
      throw new Error("Canvas 2D context is null, cannot render tree to image!");
    }
    ctx.drawImage(img, 0, 0, 4 * img.naturalWidth, 4 * img.naturalHeight);
    cvs.convertToBlob({
      type: "image/png"
    }).then((blob) => {
      const url = URL.createObjectURL(blob);
      anchor.href = url;
      anchor.download = "posterior_tree"
      anchor.click();
    });
  }
}