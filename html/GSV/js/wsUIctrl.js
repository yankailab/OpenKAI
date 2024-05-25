function $(selector) { return document.querySelector(selector); }

window.onload = function () {
    wsInit();

    var strEOJ = "EOJ";
    var dV = 0.1;

    // T
    $('#vTxInc').onclick = function (e) {
        $('#vTx').value = parseFloat($('#vTx').value) + dV;
        cmdSetTR();
    };

    $('#vTxDec').onclick = function (e) {
        $('#vTx').value = parseFloat($('#vTx').value) - dV;
        cmdSetTR();
    };

    $('#vTyInc').onclick = function (e) {
        $('#vTy').value = parseFloat($('#vTy').value) + dV;
        cmdSetTR();
    };

    $('#vTyDec').onclick = function (e) {
        $('#vTy').value = parseFloat($('#vTy').value) - dV;
        cmdSetTR();
    };

    $('#vTzInc').onclick = function (e) {
        $('#vTz').value = parseFloat($('#vTz').value) + dV;
        cmdSetTR();
    };

    $('#vTzDec').onclick = function (e) {
        $('#vTz').value = parseFloat($('#vTz').value) - dV;
        cmdSetTR();
    };


    // R
    $('#vRxInc').onclick = function (e) {
        $('#vRx').value = parseFloat($('#vRx').value) + dV;
        cmdSetTR();
    };

    $('#vRxDec').onclick = function (e) {
        $('#vRx').value = parseFloat($('#vRx').value) - dV;
        cmdSetTR();
    };

    $('#vRyInc').onclick = function (e) {
        $('#vRy').value = parseFloat($('#vRy').value) + dV;
        cmdSetTR();
    };

    $('#vRyDec').onclick = function (e) {
        $('#vRy').value = parseFloat($('#vRy').value) - dV;
        cmdSetTR();
    };

    $('#vRzInc').onclick = function (e) {
        $('#vRz').value = parseFloat($('#vRz').value) + dV;
        cmdSetTR();
    };

    $('#vRzDec').onclick = function (e) {
        $('#vRz').value = parseFloat($('#vRz').value) - dV;
        cmdSetTR();
    };


    // save TR
    $('#vTx').oninput = function (e) {
        cmdSetTR();
    };

    $('#vTy').oninput = function (e) {
        cmdSetTR();
    };

    $('#vTz').oninput = function (e) {
        cmdSetTR();
    };

    $('#vRx').oninput = function (e) {
        cmdSetTR();
    };

    $('#vRy').oninput = function (e) {
        cmdSetTR();
    };

    $('#vRz').oninput = function (e) {
        cmdSetTR();
    };

    function cmdSetTR() {
        var cmd = {
            cmd: 'setTR',
            _GeometryBase: $('#_GeometryBase').value,
            vTx: parseFloat($('#vTx').value),
            vTy: parseFloat($('#vTy').value),
            vTz: parseFloat($('#vTz').value),
            vRx: parseFloat($('#vRx').value),
            vRy: parseFloat($('#vRy').value),
            vRz: parseFloat($('#vRz').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSaveTR').onclick = function (e) {
        var cmd = {
            cmd: 'saveGeometryConfig',
            _GeometryBase: $('#_GeometryBase').value,
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


    // C
    $('#vCxInc').onclick = function (e) {
        $('#vCx').value = parseFloat($('#vCx').value) + 1;
        selectCell();
    };

    $('#vCxDec').onclick = function (e) {
        $('#vCx').value = parseFloat($('#vCx').value) - 1;
        selectCell();
    };

    $('#vCyInc').onclick = function (e) {
        $('#vCy').value = parseFloat($('#vCy').value) + 1;
        selectCell();
    };

    $('#vCyDec').onclick = function (e) {
        $('#vCy').value = parseFloat($('#vCy').value) - 1;
        selectCell();
    };

    $('#vCzInc').onclick = function (e) {
        $('#vCz').value = parseFloat($('#vCz').value) + 1;
        selectCell();
    };

    $('#vCzDec').onclick = function (e) {
        $('#vCz').value = parseFloat($('#vCz').value) - 1;
        selectCell();
    };

    $('#vCx').oninput = function (e) {
        selectCell();
    };

    $('#vCy').oninput = function (e) {
        selectCell();
    };

    $('#vCz').oninput = function (e) {
        selectCell();
    };

    function selectCell() {
        var cmd = {
            cmd: 'selectCell',
            vCx: parseFloat($('#vCx').value),
            vCy: parseFloat($('#vCy').value),
            vCz: parseFloat($('#vCz').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSetCellAlert').onclick = function (e) {
        var cmd = {
            cmd: 'setCellAlert',
            vCx: parseFloat($('#vCx').value),
            vCy: parseFloat($('#vCy').value),
            vCz: parseFloat($('#vCz').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnDelCellAlert').onclick = function (e) {
        var cmd = {
            cmd: 'delCellAlert',
            vCx: parseFloat($('#vCx').value),
            vCy: parseFloat($('#vCy').value),
            vCz: parseFloat($('#vCz').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSaveGridConfig').onclick = function (e) {
        var cmd = {
            cmd: 'saveGridConfig',
            vCx: parseFloat($('#vCx').value),
            vCy: parseFloat($('#vCy').value),
            vCz: parseFloat($('#vCz').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

};


function cmdHandler(jCmd) {
    if (jCmd.cmd == "hb") {
        $('#state').innerHTML = jCmd.s.toUpperCase();
        $('#msg').innerHTML = jCmd.msg;
    }
}
