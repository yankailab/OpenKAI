const loadBtn = $('#btnLoad');
const sendBtn = $('#btnSend');
const addCurrentPosBtn = $('#btnAddCurrentPos');
const undoBtn = $('#btnUndo');
const clearBtn = $('#btnClear');
const startBtn = $('#btnStart');
const selectionStatusEl = $('#selectionStatus');

// Layers
const vertexLayer = L.layerGroup().addTo(map);
let polygonLayer = null;
let lineLayer = null;

// Store draggable vertex markers
const vertexMarkers = [];
let currentRobotPosition = null;

// A compact visible point inside a larger touch target.
const vertexIcon = L.divIcon({
    className: 'vertex-marker',
    html: '<div class="vertex-handle"></div>',
    iconSize: [44, 44],
    iconAnchor: [22, 22]
});

function getLatLngs() {
    return vertexMarkers.map(marker => marker.getLatLng());
}

function getCoordinateArray() {
    return vertexMarkers.map(marker => {
        const ll = marker.getLatLng();
        // return {
        //     lat: Number(ll.lat.toFixed(6)),
        //     lng: Number(ll.lng.toFixed(6))
        // };
        return [
            Number(ll.lat.toFixed(6)),
            Number(ll.lng.toFixed(6))
        ];
    });
}

function updatePreview() {
    const count = vertexMarkers.length;
    sendBtn.disabled = count < 3;
    startBtn.disabled = count < 3;
    undoBtn.disabled = count === 0;
    clearBtn.disabled = count === 0;

    if (count === 0) {
        selectionStatusEl.textContent = 'No points selected';
    } else if (count < 3) {
        selectionStatusEl.textContent = count + (count === 1 ? ' point' : ' points') + ' · Add at least ' + (3 - count) + ' more';
    } else {
        selectionStatusEl.textContent = count + ' points · Ready to set';
    }
}

function redrawShape() {
    const latlngs = getLatLngs();

    if (lineLayer) {
        map.removeLayer(lineLayer);
        lineLayer = null;
    }

    if (polygonLayer) {
        map.removeLayer(polygonLayer);
        polygonLayer = null;
    }

    if (latlngs.length === 2) {
        lineLayer = L.polyline(latlngs, {
            color: 'red',
            weight: 2
        }).addTo(map);
    }

    if (latlngs.length >= 3) {
        polygonLayer = L.polygon(latlngs, {
            color: 'red',
            weight: 2,
            fillOpacity: 0.2
        }).addTo(map);
    }

    updatePreview();
}

function addVertex(latlng, redraw = true) {
    const marker = L.marker(latlng, {
        draggable: true,
        autoPan: true,
        icon: vertexIcon,
        title: 'Drag to adjust boundary point',
        alt: 'Boundary point'
    });

    marker.on('drag', redrawShape);
    marker.on('dragend', redrawShape);

    marker.addTo(vertexLayer);
    vertexMarkers.push(marker);

    if (redraw) redrawShape();
}

function updateRobotPosition(vP) {
    const valid = Array.isArray(vP) && vP.length === 2 &&
        Number.isFinite(vP[0]) && Number.isFinite(vP[1]) &&
        Math.abs(vP[0]) <= 90 && Math.abs(vP[1]) <= 180;
    currentRobotPosition = valid ? L.latLng(vP[0], vP[1]) : null;
    addCurrentPosBtn.disabled = !currentRobotPosition;
}

function addCurrentPosition() {
    if (currentRobotPosition) addVertex(currentRobotPosition);
}

function undoLastVertex() {
    const marker = vertexMarkers.pop();
    if (!marker) return;

    vertexLayer.removeLayer(marker);
    redrawShape();
}

function clearAll() {
    vertexMarkers.forEach(marker => vertexLayer.removeLayer(marker));
    vertexMarkers.length = 0;
    redrawShape();
}

function loadPolygon() {
    if (typeof wsSocket === 'undefined' || wsSocket.readyState !== WebSocket.OPEN) {
        selectionStatusEl.textContent = 'Rover disconnected. Could not load geofence.';
        return;
    }

    try {
        wsSocket.send(JSON.stringify({ cmd: 'loadGeoFence', module: 'geoFence' }) + strEOJ);
        selectionStatusEl.textContent = 'Loading saved geofence…';
    } catch (error) {
        selectionStatusEl.textContent = 'Could not send geofence load command.';
    }
}

function loadGeoFencePolygon(msg) {
    if (msg.bSuccess !== true || msg.type !== 'polygon' || !Array.isArray(msg.vPolygon)) {
        selectionStatusEl.textContent = 'Could not load a polygon geofence.';
        return;
    }

    // Validate the entire response before replacing the current editable area.
    const latlngs = msg.vPolygon.map(point => {
        if (!Array.isArray(point) || point.length !== 2 ||
            !Number.isFinite(point[0]) || !Number.isFinite(point[1]) ||
            Math.abs(point[0]) > 90 || Math.abs(point[1]) > 180) return null;
        return L.latLng(point[0], point[1]);
    });
    if (latlngs.some(point => point === null)) {
        selectionStatusEl.textContent = 'Saved geofence contains invalid coordinates.';
        return;
    }

    clearAll();
    latlngs.forEach(latlng => addVertex(latlng, false));
    redrawShape();

    if (latlngs.length === 0) {
        selectionStatusEl.textContent = 'No saved geofence. Tap the map to draw an area.';
        return;
    }

    map.fitBounds(L.latLngBounds(latlngs), { padding: [24, 24], maxZoom: 20 });
    if (latlngs.length >= 3) {
        selectionStatusEl.textContent = latlngs.length + ' points loaded · Drag to edit, then select Set to save';
    }
}

function sendPolygon() {
    if (vertexMarkers.length < 3) {
        alert('Need at least 3 points');
        return;
    }

    var cmd = {
        cmd: 'setGeoFence',
        module: 'geoFence',
        type: 'polygon',
        vPolygon: getCoordinateArray()
    };

    cmdStr = JSON.stringify(cmd) + strEOJ;
    wsSocket.send(cmdStr);

//    alert(JSON.stringify(cmdStr, null, 2));
}

// Click map to add a vertex
map.on('click', (e) => {
    addVertex(e.latlng);
});

loadBtn.addEventListener('click', loadPolygon);
addCurrentPosBtn.addEventListener('click', addCurrentPosition);
undoBtn.addEventListener('click', undoLastVertex);
clearBtn.addEventListener('click', clearAll);
sendBtn.addEventListener('click', sendPolygon);

updatePreview();



const geoFenceOverlay = {
    circle: null,
    shaft: null,
    headL: null,
    headR: null,
    lastMsg: null
  };

  function ensureGeoFenceOverlay() {
    if (geoFenceOverlay.circle) return;

    geoFenceOverlay.circle = L.circleMarker([0, 0], {
      radius: 6,
      color: '#169c2f',
      weight: 2,
      fillColor: '#169c2f',
      fillOpacity: 0.35
    }).addTo(map);

    geoFenceOverlay.shaft = L.polyline([], {
      color: '#169c2f',
      weight: 3,
      lineCap: 'round'
    }).addTo(map);

    geoFenceOverlay.headL = L.polyline([], {
      color: '#169c2f',
      weight: 3,
      lineCap: 'round'
    }).addTo(map);

    geoFenceOverlay.headR = L.polyline([], {
      color: '#169c2f',
      weight: 3,
      lineCap: 'round'
    }).addTo(map);
  }

  function parseLatLng(v) {
    if (!Array.isArray(v) || v.length < 2) return null;

    const lat = Number(v[0]);
    const lng = Number(v[1]);

    if (!Number.isFinite(lat) || !Number.isFinite(lng)) return null;
    return L.latLng(lat, lng);
  }

  function buildArrowShape(startLatLng, endLatLng) {
    const p0 = map.latLngToLayerPoint(startLatLng);
    const p1 = map.latLngToLayerPoint(endLatLng);

    const dx = p1.x - p0.x;
    const dy = p1.y - p0.y;
    const len = Math.hypot(dx, dy);

    // If too short, just draw the shaft.
    if (len < 1) {
      return {
        shaft: [startLatLng, endLatLng],
        headL: [endLatLng, endLatLng],
        headR: [endLatLng, endLatLng]
      };
    }

    const ang = Math.atan2(dy, dx);

    // Keep the arrowhead size visually reasonable in pixels.
    const headLen = Math.max(10, Math.min(18, len * 0.35));
    const spread = 28 * Math.PI / 180;

    const leftPt = L.point(
      p1.x + headLen * Math.cos(ang + Math.PI - spread),
      p1.y + headLen * Math.sin(ang + Math.PI - spread)
    );

    const rightPt = L.point(
      p1.x + headLen * Math.cos(ang + Math.PI + spread),
      p1.y + headLen * Math.sin(ang + Math.PI + spread)
    );

    return {
      shaft: [startLatLng, endLatLng],
      headL: [endLatLng, map.layerPointToLatLng(leftPt)],
      headR: [endLatLng, map.layerPointToLatLng(rightPt)]
    };
  }

  function updateGeoFenceOverlay(msg) {
    if (!msg || msg.cmd !== 'geoFence') return;

    const vP = parseLatLng(msg.vP);
    const vPnext = parseLatLng(msg.vPnext);
    if (!vP || !vPnext) return;

    ensureGeoFenceOverlay();
    geoFenceOverlay.lastMsg = msg;

    const color = msg.bBreach ? '#d61f1f' : '#169c2f';
    const shape = buildArrowShape(vP, vPnext);

    geoFenceOverlay.circle
      .setLatLng(vP)
      .setStyle({
        color,
        fillColor: color
      });

    geoFenceOverlay.shaft
      .setLatLngs(shape.shaft)
      .setStyle({ color });

    geoFenceOverlay.headL
      .setLatLngs(shape.headL)
      .setStyle({ color });

    geoFenceOverlay.headR
      .setLatLngs(shape.headR)
      .setStyle({ color });

    geoFenceOverlay.circle.bringToFront();
    geoFenceOverlay.shaft.bringToFront();
    geoFenceOverlay.headL.bringToFront();
    geoFenceOverlay.headR.bringToFront();
  }

  // Recompute arrowhead after zoom so its size stays visually similar.
  function redrawGeoFenceOverlay() {
    if (geoFenceOverlay.lastMsg) {
      updateGeoFenceOverlay(geoFenceOverlay.lastMsg);
    }
  }

  map.on('zoomend', redrawGeoFenceOverlay);
  map.on('resize', redrawGeoFenceOverlay);
