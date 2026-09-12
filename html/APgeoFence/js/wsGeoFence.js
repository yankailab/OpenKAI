const controlIntervalMs = 100;
let activeControl = null;
let controlTimer = null;

window.onload = function () {
    bindControlButton('#btnForward', 'F', 'Forward');
    bindControlButton('#btnLeft', 'L', 'Left');
    bindControlButton('#btnRight', 'R', 'Right');
    bindControlButton('#btnBack', 'B', 'Backward');
    bindControlButton('#btnStop', 'S', 'STOP');
    $('#btnStart').addEventListener('click', onStartClick);
    $('#btnCutterOn').addEventListener('click', () => onCutterClick(1));
    $('#btnCutterOff').addEventListener('click', () => onCutterClick(0));

    window.addEventListener('pointerup', endControlPointer);
    window.addEventListener('pointercancel', endControlPointer);
    window.addEventListener('blur', stopControl);
    window.addEventListener('pagehide', stopControl);
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) stopControl();
    });

    wsInit();
    wsSocket.addEventListener('close', stopControl);
    wsSocket.addEventListener('error', stopControl);
};

function bindControlButton(selector, btn, label) {
    const button = $(selector);
    button.addEventListener('pointerdown', event => {
        if (event.button !== 0) return;
        event.preventDefault();
        stopControl();
        activeControl = { button, btn, pointerId: event.pointerId };
        button.setPointerCapture(event.pointerId);
        button.classList.add('is-held');
        $('#controlFeedback').textContent = label + ' · Sending while held';
        if (sendControl()) controlTimer = setInterval(sendControl, controlIntervalMs);
    });

    // Touch pointers are captured, so pointerleave alone does not detect a
    // finger sliding outside the button. Check the bounds during capture too.
    button.addEventListener('pointermove', event => {
        if (!activeControl || event.pointerId !== activeControl.pointerId) return;
        const rect = button.getBoundingClientRect();
        if (event.buttons === 0 || event.clientX < rect.left || event.clientX >= rect.right ||
            event.clientY < rect.top || event.clientY >= rect.bottom) stopControl();
    });
    button.addEventListener('pointerleave', endControlPointer);
    button.addEventListener('lostpointercapture', endControlPointer);
    button.addEventListener('contextmenu', event => event.preventDefault());
}

function endControlPointer(event) {
    if (activeControl && event.pointerId === activeControl.pointerId) stopControl();
}

function stopControl() {
    clearInterval(controlTimer);
    controlTimer = null;
    const control = activeControl;
    activeControl = null;
    if (!control) return;
    control.button.classList.remove('is-held');
    if (control.button.hasPointerCapture(control.pointerId)) {
        control.button.releasePointerCapture(control.pointerId);
    }
    $('#controlFeedback').textContent = 'Control released. No commands being sent.';
}

function sendControl() {
    if (!activeControl) return false;
    if (typeof wsSocket === 'undefined' || wsSocket.readyState !== WebSocket.OPEN) {
        stopControl();
        $('#controlFeedback').textContent = 'Rover disconnected. No command sent.';
        return false;
    }

    try {
        wsSocket.send(JSON.stringify({ cmd: 'ctrlBtn', module: 'apDrive', btn: activeControl.btn }) + strEOJ);
        return true;
    } catch (error) {
        stopControl();
        $('#controlFeedback').textContent = 'Could not send rover control.';
        return false;
    }
}

function onStartClick() {
    stopControl();
    if (typeof wsSocket === 'undefined' || wsSocket.readyState !== WebSocket.OPEN) {
        $('#controlFeedback').textContent = 'Rover disconnected. No command sent.';
        return;
    }

    try {
        wsSocket.send(JSON.stringify({ cmd: 'startAuto', module: 'apDrive' }) + strEOJ);
        $('#controlFeedback').textContent = 'Auto mode start command sent.';
    } catch (error) {
        $('#controlFeedback').textContent = 'Could not send auto mode start command.';
    }
}


function onCutterClick(bServoON) {
    const feedback = $('#cutterFeedback');
    const isJapanese = document.documentElement.lang === 'ja';
    if (typeof wsSocket === 'undefined' || wsSocket.readyState !== WebSocket.OPEN) {
        feedback.textContent = isJapanese ? 'ローバーが未接続のため、コマンドを送信できません。' : 'Rover disconnected. No command sent.';
        return;
    }

    try {
        wsSocket.send(JSON.stringify({ cmd: 'setServo', module: 'apDrive', bServoON }) + strEOJ);
        const state = bServoON ? 'ON' : 'OFF';
        feedback.textContent = isJapanese ? `カッター${state}コマンドを送信しました。` : `Cutter ${state} command sent.`;
    } catch (error) {
        feedback.textContent = isJapanese ? 'カッターの操作コマンドを送信できませんでした。' : 'Could not send cutter command.';
    }
}

function cmdHandler(event) {
    $('#cmdState').value = (event.data + "\n\n" + $('#cmdState').value).slice(0, 12000);

    const jCmd = JSON.parse(event.data);
    if (jCmd.cmd == 'geoFence') {
        updateRobotPosition(jCmd.vP);
        updateGeoFenceOverlay(jCmd);
    } else if (jCmd.cmd == 'loadGeoFence') {
        loadGeoFencePolygon(jCmd);
    }
};
