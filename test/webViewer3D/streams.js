// Executed by browser.py in Chromium. Exercise stream lifetime at the renderer.
const THREE = await import('/vendor/three.module.min.js');
const { Viewer3D } = await import('/js/viewer3D.js');
const container = document.createElement('div');
container.style.cssText = 'position:fixed;width:128px;height:128px';
document.body.append(container);
const viewer = new Viewer3D(container);
const check = (ok, message) => { if (!ok) throw Error(message); };
viewer.configure({ objects: [{ id: 7, name: 'grid' }], background: [0, 0, 0], showGrid: false,
  autoBound: true, camera: { eye: [0, 0, 5], target: [0, 0, 0], up: [0, 1, 0], type: 0, fov: 60, near: .1, far: 100 } });
viewer.axes.visible = false;
const cells = new Uint8Array(20); cells.set([0, 255, 0, 128], 16);
const cellFrame = { type: 'cells', objects: [{ id: 7, count: 1, pointSize: 1, opacity: 1,
  bounds: [-1, -1, -1, 1, 1, 1], grid: { origin: [0, 0, 0], size: [2, 2, 2], maxLevel: 2, cells } }] };
const pointFrame = { type: 'points', objects: [{ id: 7, count: 1, pointSize: 4, opacity: 1,
  bounds: [4, 0, 0, 4, 0, 0], positions: new Float32Array([4, 0, 0]), colors: new Uint8Array([255, 0, 0, 128]) }] };
const lineFrame = { type: 'lines', objects: [{ id: 7, count: 1, pointSize: 1, opacity: 1,
  bounds: [-4, 0, 0, 0, 0, 0], positions: new Float32Array([-4, 0, 0, 0, 0, 0]), colors: new Uint8Array([0, 0, 255, 255, 0, 0, 255, 255]) }] };

// No point or line configuration/frame is needed to display or pick cells.
viewer.update(cellFrame); viewer.render();
const object = viewer.objects.get(7), boxes = object.boxes;
check(viewer.renderer.info.render.lines === 12 && !viewer.bounds.isEmpty(), 'Cells-only scene is blank');
const ray = new THREE.Ray(new THREE.Vector3(0, 0, 5), new THREE.Vector3(0, 0, -1));
const hit = viewer.picker.pickRay(ray);
check(hit && hit.depth === 0, 'Cells-only scene is not pickable');
viewer.picker.toggle(hit.source, hit.id);
viewer.update(pointFrame); viewer.update(lineFrame);
check(viewer.objects.size === 1 && object.streams.size === 3 && object.boxes === boxes, 'Shared source components were replaced');
check(viewer.bounds.min.x === -4 && viewer.bounds.max.x === 4, 'Bounds do not include all three streams');
const camera = viewer.camera.position.clone();
viewer.clearStream('points'); viewer.update({ type: 'lines', objects: [] }); viewer.render();
check(object.streams.size === 1 && boxes.geometry.instanceCount === 1 && viewer.picker.count === 1,
  'Missing point/line streams removed cells or selections');
check(viewer.bounds.min.x === -1 && viewer.bounds.max.x === 1, 'Removed streams left stale bounds');
check(viewer.camera.position.distanceTo(camera) < 1e-8, 'Disconnect moved the camera');
viewer.setGridSolid(true); viewer.setGridLevelRange(1, 40); viewer.setVisible(7, false);
viewer.update(pointFrame); viewer.update(lineFrame);
check(!boxes.visible && !object.points.visible && !object.lines.visible, 'A new stream ignored source visibility');
check(boxes.geometry.instanceCount === 0 && boxes.solid.visible && viewer.picker.count === 1, 'New streams reset grid controls or selections');
check(viewer.camera.position.distanceTo(camera) < 1e-8, 'Reconnect refitted the camera');
viewer.setVisible(7, true);
viewer.clearStream('cells');
check(viewer.objects.get(7) === object && object.points.geometry.drawRange.count === 1 && object.lines.geometry.drawRange.count === 2,
  'Cells disconnect removed point/line geometry');
check(viewer.picker.count === 1 && viewer.picker.pickRay(ray), 'Cells disconnect lost retained selection');
viewer.update(cellFrame);
check(viewer.picker.count === 1 && boxes.geometry.instanceCount === 0 && boxes.solid.visible, 'Cells reconnect reset controls or selection');
viewer.clearStream('points'); viewer.clearStream('cells');
check(viewer.objects.get(7) === object && object.streams.size === 1, 'Lines-only source was removed');
viewer.clearStream('lines');
check(viewer.objects.size === 0 && viewer.bounds.isEmpty() && viewer.picker.count === 1, 'Final source removal/disposal lost selection');
viewer.update(pointFrame); viewer.render();
check(viewer.objects.get(7).points.geometry.drawRange.count === 1, 'Points-only source failed to return');
viewer.dispose(); container.remove();
return 'PASS: cells-only rendering/picking, independent geometry updates/removal, shared source bounds, reconnect camera/controls/selection persistence';
