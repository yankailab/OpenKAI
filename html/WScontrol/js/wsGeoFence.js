window.onload = function () {
    wsInit();

    // controls
    // $('#btnForward').onmousedown = function (e) {
    //     cmdSetSteerSpeed(0, 1);
    // };

    // function cmdSetSteerSpeed(setSteer, setSpeed) {
    //     var cmd = {
    //         cmd: 'setSteerSpeed',
    //         module: 'apDrive',
    //         steer: setSteer,
    //         speed: setSpeed,
    //     };

    //     cmdStr = JSON.stringify(cmd) + strEOJ;
    //     wsSocket.send(cmdStr);
    // };
};


function cmdHandler(event) {
    $('#txtMsg').value = event.data + "\n\n" + $('#txtMsg').value;

    jCmd = JSON.parse(event.data);
    if (jCmd.cmd == 'geoFence') {
        updateGeoFenceOverlay(jCmd);
    }
};


const jsonPreviewEl = $('#jsonPreview');
const sendBtn = $('#btnSend');
const undoBtn = $('#btnUndo');
const clearBtn = $('#btnClear');

// Layers
const vertexLayer = L.layerGroup().addTo(map);
let polygonLayer = null;
let lineLayer = null;

// Store draggable vertex markers
const vertexMarkers = [];

// Small draggable handle icon
const vertexIcon = L.divIcon({
    className: '',
    html: '<div class="vertex-handle"></div>',
    iconSize: [18, 18],
    iconAnchor: [9, 9]
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
    // const payload = {
    //     type: 'polygon',
    //     coordinates: getCoordinateArray()
    // };
    // jsonPreviewEl.textContent = JSON.stringify(payload, null, 2);
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

function addVertex(latlng) {
    const marker = L.marker(latlng, {
        draggable: true,
        autoPan: true,
        icon: vertexIcon
    });

    marker.on('drag', redrawShape);
    marker.on('dragend', redrawShape);

    marker.addTo(vertexLayer);
    vertexMarkers.push(marker);

    redrawShape();
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