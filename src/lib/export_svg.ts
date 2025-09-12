export function export_svg(svg : SVGElement, cvs : OffscreenCanvas, anchor : HTMLAnchorElement) {
  const svg_xml = (new XMLSerializer()).serializeToString(svg);
  const uri = `data:image/svg+xml;charset=utf-8,${encodeURIComponent(svg_xml)}`;
  const img = new Image();
  img.src = uri;
  img.onload = () => {
    cvs.width = img.naturalWidth;
    cvs.height = img.naturalHeight;
    cvs.getContext('2d').drawImage(img, 0, 0, img.naturalWidth, img.naturalHeight);
    cvs.convertToBlob({
      type: "image/png"
    }).then((blob) => {
      const url = URL.createObjectURL(blob);
      anchor.href = url;
      anchor.download = "posterior_tree.png"
      anchor.click();
    });
  }
}