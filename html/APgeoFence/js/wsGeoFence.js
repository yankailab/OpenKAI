window.onload = function () {
    $('#btnForward').addEventListener('click', onForwardClick);
    $('#btnLeft').addEventListener('click', onLeftClick);
    $('#btnRight').addEventListener('click', onRightClick);
    $('#btnBack').addEventListener('click', onBackwardClick);
    $('#btnStop').addEventListener('click', onStopClick);
    $('#btnStart').addEventListener('click', onStartClick);

    wsInit();
};

// Command templates: replace the placeholder feedback when the rover protocol
// is implemented. These handlers do not send commands or change rover state.
function onForwardClick() {
    // TODO: Send the manual forward command.
    showPendingControl('Forward');
}

function onLeftClick() {
    // TODO: Send the manual left command.
    showPendingControl('Left');
}

function onRightClick() {
    // TODO: Send the manual right command.
    showPendingControl('Right');
}

function onBackwardClick() {
    // TODO: Send the manual backward command.
    showPendingControl('Backward');
}

function onStopClick() {
    // TODO: Send an emergency stop that overrides manual and auto movement.
    showPendingControl('STOP');
}

function onStartClick() {
    // TODO: Start auto mode using the geofence set on the rover.
    showPendingControl('Auto mode');
}

function showPendingControl(action) {
    $('#controlFeedback').textContent = action + ' is not connected yet. No command sent.';
}


function cmdHandler(event) {
    $('#cmdState').value = event.data + "\n\n" + $('#cmdState').value;

    jCmd = JSON.parse(event.data);
    if (jCmd.cmd == 'geoFence') {
        updateGeoFenceOverlay(jCmd);
    }
};
