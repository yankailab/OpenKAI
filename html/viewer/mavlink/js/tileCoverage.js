// Explicit availability distinguishes intentional geographic holes from broken
// cached files. Coordinates use XYZ/Web Mercator and inclusive Y ranges.
export function tileCoverage(document) {
  const object = value => value !== null && typeof value === 'object' && !Array.isArray(value);
  const index = value => /^(0|[1-9][0-9]*)$/.test(value) && Number.isSafeInteger(Number(value));
  const { minimumLevel, maximumLevel, levels } = document || {};
  if (!object(document) || document.format !== 'openkai-tile-coverage/1' ||
      !Number.isInteger(minimumLevel) || !Number.isInteger(maximumLevel) ||
      minimumLevel < 0 || maximumLevel > 23 || minimumLevel > maximumLevel || !object(levels))
    throw new Error('Invalid downloaded tile coverage');
  const available = new Map();
  for (const [zoom, columns] of Object.entries(levels)) {
    const level = Number(zoom), count = 2 ** level;
    if (!index(zoom) || level < minimumLevel || level > maximumLevel || !object(columns))
      throw new Error('Invalid coverage level');
    const rows = new Map();
    for (const [x, ranges] of Object.entries(columns)) {
      if (!index(x) || Number(x) >= count || !Array.isArray(ranges)) throw new Error('Invalid coverage column');
      let previousEnd = -1;
      for (const range of ranges) {
        if (!Array.isArray(range) || range.length !== 2 || !range.every(Number.isInteger) ||
            range[0] <= previousEnd || range[0] > range[1] || range[1] >= count)
          throw new Error('Invalid coverage row range');
        previousEnd = range[1];
      }
      rows.set(Number(x), ranges);
    }
    available.set(level, rows);
  }
  for (let level = minimumLevel; level <= maximumLevel; ++level)
    if (!available.has(level)) throw new Error('Missing coverage level');
  return (x, y, level) => {
    if (![x, y, level].every(Number.isInteger)) return false;
    const ranges = available.get(level)?.get(x);
    let left = 0, right = (ranges?.length || 0) - 1;
    while (left <= right) {
      const middle = Math.floor((left + right) / 2), [start, end] = ranges[middle];
      if (y < start) right = middle - 1;
      else if (y > end) left = middle + 1;
      else return true;
    }
    return false;
  };
}

let transparent;
export function transparentTile() {
  if (!transparent) {
    const canvas = document.createElement('canvas');
    canvas.width = canvas.height = 256;
    transparent = Promise.resolve(canvas);
  }
  return transparent;
}
