import { cellBox, CELL_BYTES } from './octreeCells.js';

export function cellIDKey(id) {
  return Array.from(id, byte => byte.toString(16).padStart(2, '0')).join('');
}

export function childID(parent, child) {
  const id = parent.slice(), depth = id[0] & 63;
  if (depth >= 40) throw new Error('Maximum grid depth reached');
  const bit = 123 - depth * 3, byte = bit >> 3, shift = bit % 8;
  id[byte] |= (child << shift) & 255;
  if (shift > 5) id[byte + 1] |= child >> (8 - shift);
  id[0] = (id[0] & 192) | (depth + 1);
  return id;
}

export function selectionGrid(header, ids) {
  const cells = new Uint8Array(ids.size * CELL_BYTES);
  let at = 0;
  for (const id of ids.values()) {
    cells.set(id, at);
    cells[at + 16] = 255; // selected cells are red
    at += CELL_BYTES;
  }
  return { ...header, cells };
}

export function rootBounds(header) {
  return [...header.origin.map((x, axis) => x - header.size[axis] / 2),
    ...header.origin.map((x, axis) => x + header.size[axis] / 2)];
}

export function sameGridHeader(a, b) {
  return a.maxLevel === b.maxLevel && a.origin.every((x, i) => x === b.origin[i]) &&
    a.size.every((x, i) => x === b.size[i]);
}

// Selected cells share an octree, so their volumes are disjoint or nested.
// Remove descendants of selected ancestors before calculating union volumes.
function selectedRegions(header, ids) {
  const tree = {};
  for (const id of ids.values()) {
    let node = tree;
    for (let level = 0; level < (id[0] & 63) && !node.selected; ++level) {
      const bit = 123 - level * 3, byte = bit >> 3, shift = bit % 8;
      const child = ((id[byte] >> shift) | ((id[byte + 1] || 0) << (8 - shift))) & 7;
      node = (node.children ??= {})[child] ??= {};
    }
    if (node.selected) continue;
    node.selected = true;
    node.id = id;
    delete node.children;
  }
  const regions = [];
  const visit = node => {
    if (node.selected) {
      const grid = selectionGrid(header, new Map([[cellIDKey(node.id), node.id]]));
      const { center, size } = cellBox(grid, 0);
      regions.push({ min: center.map((x, i) => x - size[i] / 2), max: center.map((x, i) => x + size[i] / 2) });
    } else for (const child of Object.values(node.children || {})) visit(child);
  };
  visit(tree);
  return regions;
}

// Cover the old selected volume in the new root with the smallest available
// cells along its boundary. Fully covered subtrees collapse into a single cell.
// Keep boundary cells even for a small overlap: a coarser new root must not
// silently erase an in-bounds selection. A work budget prevents deep, misaligned
// boundaries from freezing the UI; remaining boundary nodes stay coarser.
export function remapSelection(oldHeader, newHeader, ids, maxNodes = 100000) {
  if (sameGridHeader(oldHeader, newHeader)) return new Map(ids);
  const result = new Map(), regions = selectedRegions(oldHeader, ids);
  let nodes = 0;
  const visit = (id, center, size, candidates) => {
    ++nodes;
    const min = center.map((x, i) => x - size[i] / 2), max = center.map((x, i) => x + size[i] / 2);
    const volume = size[0] * size[1] * size[2];
    const intersecting = [];
    let covered = 0;
    for (const region of candidates) {
      let overlap = 1;
      for (let axis = 0; axis < 3; ++axis)
        overlap *= Math.max(0, Math.min(max[axis], region.max[axis]) - Math.max(min[axis], region.min[axis]));
      if (overlap > 0) { covered += overlap; intersecting.push(region); }
    }
    if (!covered) return;
    if (covered >= volume * (1 - 1e-12) || (id[0] & 63) >= newHeader.maxLevel || nodes >= maxNodes) {
      result.set(cellIDKey(id), id);
      return;
    }
    const childSize = size.map(x => x / 2);
    for (let child = 0; child < 8; ++child)
      visit(childID(id, child), center.map((x, i) => x + size[i] * (child & (4 >> i) ? .25 : -.25)), childSize, intersecting);
  };
  visit(new Uint8Array(16), newHeader.origin, newHeader.size, regions);
  return result;
}
