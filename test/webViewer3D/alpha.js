// Executed by browser.py in Chromium against the actual viewer materials.
const THREE = await import('/vendor/three.module.min.js');
const { Viewer3D } = await import('/js/viewer3D.js');
const container = document.createElement('div');
container.style.cssText = 'position:fixed;width:128px;height:128px';
document.body.append(container);
const viewer = new Viewer3D(container);
viewer.renderer.setPixelRatio(1);
viewer.renderer.setSize(128, 128);
viewer.scene.background.setRGB(0, 0, 0);
viewer.axes.visible = viewer.grid.visible = false;
viewer.camera = new THREE.PerspectiveCamera(60, 1, .1, 100);
viewer.camera.position.set(3, 2, 5);
viewer.camera.lookAt(0, 0, 0);
const pixels = new Uint8Array(128 * 128 * 4), gl = viewer.renderer.getContext();
function check(ok, message) { if (!ok) throw Error(message); }
function render(kind, alpha, opacity = 1) {
  const colors = new Uint8Array([255, 255, 255, alpha]);
  const cells = new Uint8Array(20); cells.set(colors, 16);
  const type = kind === 'boxes' ? 'cells' : kind;
  for (const other of ['points', 'lines', 'cells']) if (other !== type) viewer.clearStream(other);
  viewer.update({ type, objects: [{ id: 0, count: 1, pointSize: 12, opacity,
    bounds: [-1, -1, -1, 1, 1, 1],
    positions: new Float32Array(kind === 'lines' ? [-1, 0, 0, 1, 0, 0] : [0, 0, 0]),
    colors: kind === 'lines' ? new Uint8Array([...colors, ...colors]) : colors,
    grid: { origin: [0, 0, 0], size: [2, 2, 2], maxLevel: 1, cells }
  }] });
  const mesh = viewer.objects.get(0)[kind];
  check(mesh.material.transparent === (alpha < 255 || opacity < 1), `${kind}: incorrect blending state`);
  check(mesh.material.depthWrite === !mesh.material.transparent, `${kind}: incorrect depth writes`);
  viewer.renderer.render(viewer.scene, viewer.camera);
  gl.readPixels(0, 0, 128, 128, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
  return Uint8Array.from({ length: 128 * 128 }, (_, i) => pixels[i * 4]);
}
let wirePixels = 0;
for (const [kind, solid] of [['points', false], ['lines', false], ['boxes', false], ['boxes', true]]) {
  viewer.setGridSolid(solid);
  const zero = render(kind, 0), half = render(kind, 128), full = render(kind, 255);
  check(zero.every(x => x === 0), `${kind}: alpha zero remains visible`);
  check(half.some(x => x > 0) && Math.max(...half) < 255, `${kind}: fractional alpha ignored`);
  check(Math.max(...full) === 255, `${kind}: opaque geometry faded`);
  if (kind === 'boxes') {
    if (!solid) wirePixels = full.filter(x => x > 0).length;
    else {
      check(full.filter(x => x > 0).length > wirePixels * 3, 'Solid mode did not fill box faces');
      check(viewer.renderer.info.render.triangles === 12 && viewer.renderer.info.render.lines === 0, 'Solid mode drew wire edges');
    }
  }
  const multiplied = render(kind, 128, .5), quarter = render(kind, 64);
  check(multiplied.every((x, i) => Math.abs(x - quarter[i]) <= 2), `${kind}: object opacity did not multiply alpha`);
}
// Mixed alphas in one primitive buffer must enable blending, even if first is opaque.
render('points', 255);
let object = viewer.objects.get(0);
viewer.update({ type: 'points', objects: [{ id: 0, count: 2, pointSize: 12, opacity: 1,
  bounds: [-1, -1, -1, 1, 1, 1], positions: new Float32Array([-1, 0, 0, 1, 0, 0]),
  colors: new Uint8Array([255, 255, 255, 255, 255, 255, 255, 0]) }] });
check(object.points.material.transparent && !object.points.material.depthWrite, 'Mixed point alphas ignored');

// Two translucent cells entered near-first must blend in camera depth order,
// retaining ID/color correspondence for picking after the camera reverses.
const { childID, cellIDKey } = await import('/js/gridSelection.js');
const nearID = childID(new Uint8Array(16), 7), farID = childID(new Uint8Array(16), 6);
const cells = new Uint8Array(40);
cells.set(nearID); cells.set([255, 0, 0, 128], 16);
cells.set(farID, 20); cells.set([0, 0, 255, 128], 36);
viewer.clearStream('points');
viewer.update({ type: 'cells', objects: [{ id: 0, count: 2, pointSize: 1, opacity: 1,
  bounds: [-1, -1, -1, 1, 1, 1],
  grid: { origin: [0, 0, 0], size: [2, 2, 2], maxLevel: 1, cells } }] });
object = viewer.objects.get(0);
for (const side of [1, -1]) {
  viewer.camera.position.set(.5, .5, side * 5);
  viewer.camera.lookAt(.5, .5, 0);
  viewer.camera.updateMatrixWorld(); viewer.scene.updateMatrixWorld();
  object.boxes.sortCells(viewer.camera);
  check(cellIDKey(object.boxes.getCell(0).id) === cellIDKey(side === 1 ? farID : nearID), 'Solid cells not sorted far-to-near');
  const version = object.boxes.geometry.getAttribute('cellColor').version;
  object.boxes.sortCells(viewer.camera);
  check(object.boxes.geometry.getAttribute('cellColor').version === version, 'Stationary camera re-uploaded sorted cells');
  viewer.renderer.render(viewer.scene, viewer.camera);
  const pixel = new Uint8Array(4); gl.readPixels(64, 64, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixel);
  check(Math.abs(pixel[side === 1 ? 0 : 2] - 128) <= 2 && Math.abs(pixel[side === 1 ? 2 : 0] - 64) <= 2, `Incorrect solid blend: ${pixel}`);
  const ray = new THREE.Ray(viewer.camera.position.clone(), new THREE.Vector3(0, 0, -side));
  check(cellIDKey(viewer.picker.pickRay(ray).id) === cellIDKey(side === 1 ? nearID : farID), 'Sorting changed picked ID');
}
viewer.setGridSolid(false);
viewer.renderer.render(viewer.scene, viewer.camera);
check(viewer.renderer.info.render.lines === 24 && viewer.renderer.info.render.triangles === 0, 'Switching back left solid faces');
viewer.setGridSolid(true); viewer.setGridLevelRange(2, 40);
viewer.renderer.render(viewer.scene, viewer.camera);
check(viewer.renderer.info.render.triangles === 0, 'Filtered solid cells still rendered');
viewer.dispose(); container.remove();
return 'PASS: point/line/wire/solid alpha, opacity multiplication, mode switching, camera-depth sorting, exact picking and level filtering';
