export const CELL_BYTES = 19;

export function validateCellIDs(cells, maxLevel) {
  for (let at = 0; at < cells.length; at += CELL_BYTES) {
    const depth = cells[at] & 63;
    if (depth > maxLevel || (cells[at + 15] & 192)) throw new Error('Invalid grid cell ID');
    // All bits below the used path, except the six depth bits, must be zero.
    const end = 126 - depth * 3;
    for (let bit = 6; bit < end;) {
      const n = Math.min(8 - (bit % 8), end - bit);
      if ((cells[at + (bit >> 3)] >> (bit % 8)) & ((1 << n) - 1))
        throw new Error('Noncanonical grid cell ID');
      bit += n;
    }
  }
}

// IDs stay as bytes, including paths crossing the 64-bit boundary and depth 40.
export function cellBox(grid, index) {
  const at = index * CELL_BYTES, center = [...grid.origin], size = [...grid.size];
  const depth = grid.cells[at] & 63;
  for (let level = 0; level < depth; ++level) {
    const bit = 123 - level * 3, byte = at + (bit >> 3), shift = bit % 8;
    const child = ((grid.cells[byte] >> shift) | ((grid.cells[byte + 1] || 0) << (8 - shift))) & 7;
    for (let axis = 0; axis < 3; ++axis) {
      center[axis] += size[axis] * ((child & (4 >> axis)) ? 0.25 : -0.25);
      size[axis] *= 0.5;
    }
  }
  return { id: grid.cells.subarray(at, at + 16), center, size,
    color: grid.cells.subarray(at + 16, at + CELL_BYTES) };
}
