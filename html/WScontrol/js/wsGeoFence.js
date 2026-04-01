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
    if (jCmd.cmd == 'update') {

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
        return {
            lat: Number(ll.lat.toFixed(6)),
            lng: Number(ll.lng.toFixed(6))
        };
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
        module: 'apDrive',
        type: 'polygon',
        coordinates: getCoordinateArray()
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