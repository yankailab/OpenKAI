function $(selector) { return document.querySelector(selector); }

window.onload = function () {
    wsInit();

    var strEOJ = "EOJ";
    var dV = 0.1;

    // grid
    $('#btnUpdateGrid').onclick = function (e) {
        var cmd = {
            cmd: 'updateGrid',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSetGrid').onclick = function (e) {
        var cmd = {
            cmd: 'setGrid',
            pOx: parseFloat($('#pOx').value),
            pOy: parseFloat($('#pOy').value),
            pOz: parseFloat($('#pOz').value),
            gSize: parseFloat($('#gSize').value),
            cSize: parseFloat($('#cSize').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSaveGrid').onclick = function (e) {
        var cmd = {
            cmd: 'saveGrid',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };
    

    // update config
    $('#btnUpdateTR').onclick = function (e) {
        var cmd = {
            cmd: 'updateTR',
            _GeometryBase: $('#_GeometryBase').value,
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


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



    // set TR all
    // T
    $('#vTxAllInc').onclick = function (e) {
        $('#vTxAll').value = parseFloat($('#vTxAll').value) + dV;
        cmdSetTRall();
    };

    $('#vTxAllDec').onclick = function (e) {
        $('#vTxAll').value = parseFloat($('#vTxAll').value) - dV;
        cmdSetTRall();
    };

    $('#vTyAllInc').onclick = function (e) {
        $('#vTyAll').value = parseFloat($('#vTyAll').value) + dV;
        cmdSetTRall();
    };

    $('#vTyAllDec').onclick = function (e) {
        $('#vTyAll').value = parseFloat($('#vTyAll').value) - dV;
        cmdSetTRall();
    };

    $('#vTzAllInc').onclick = function (e) {
        $('#vTzAll').value = parseFloat($('#vTzAll').value) + dV;
        cmdSetTRall();
    };

    $('#vTzAllDec').onclick = function (e) {
        $('#vTzAll').value = parseFloat($('#vTzAll').value) - dV;
        cmdSetTRall();
    };


    // R
    $('#vRxAllInc').onclick = function (e) {
        $('#vRxAll').value = parseFloat($('#vRxAll').value) + dV;
        cmdSetTRall();
    };

    $('#vRxAllDec').onclick = function (e) {
        $('#vRxAll').value = parseFloat($('#vRxAll').value) - dV;
        cmdSetTRall();
    };

    $('#vRyAllInc').onclick = function (e) {
        $('#vRyAll').value = parseFloat($('#vRyAll').value) + dV;
        cmdSetTRall();
    };

    $('#vRyAllDec').onclick = function (e) {
        $('#vRyAll').value = parseFloat($('#vRyAll').value) - dV;
        cmdSetTRall();
    };

    $('#vRzAllInc').onclick = function (e) {
        $('#vRzAll').value = parseFloat($('#vRzAll').value) + dV;
        cmdSetTRall();
    };

    $('#vRzAllDec').onclick = function (e) {
        $('#vRzAll').value = parseFloat($('#vRzAll').value) - dV;
        cmdSetTRall();
    };

    // set TR all
    $('#vTxAll').oninput = function (e) {
        cmdSetTRall();
    };

    $('#vTyAll').oninput = function (e) {
        cmdSetTRall();
    };

    $('#vTzAll').oninput = function (e) {
        cmdSetTRall();
    };

    $('#vRxAll').oninput = function (e) {
        cmdSetTRall();
    };

    $('#vRyAll').oninput = function (e) {
        cmdSetTRall();
    };

    $('#vRzAll').oninput = function (e) {
        cmdSetTRall();
    };

    function cmdSetTRall() {
        var cmd = {
            cmd: 'setTRall',
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

    $('#btnSaveTRall').onclick = function (e) {
        var cmd = {
            cmd: 'saveGeometryConfigAll',
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

    // params
    $('#nPs').oninput = function (e) {
        cmdSetParams();
    };

    $('#nPsInc').onclick = function (e) {
        $('#nPs').value = parseFloat($('#nPs').value) + 1;
        cmdSetParams();
    };

    $('#nPsDec').onclick = function (e) {
        $('#nPs').value = parseFloat($('#nPs').value) - 1;
        cmdSetParams();
    };

    function cmdSetParams() {
        var cmd = {
            cmd: 'setParams',
            nPs: parseFloat($('#nPs').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


};

function cmdHandler(jCmd) {
    if (jCmd.cmd == "updateTR")
    {
        $('#_GeometryBase').value = jCmd._GeometryBase;
        $('#vTx').value = jCmd.vTx;
        $('#vTy').value = jCmd.vTy;
        $('#vTz').value = jCmd.vTz;
        $('#vRx').value = jCmd.vRx;
        $('#vRy').value = jCmd.vRy;
        $('#vRz').value = jCmd.vRz;
    }
    else if (jCmd.cmd == "updateGrid")
    {
        $('#pOx').value = jCmd.pOx;
        $('#pOy').value = jCmd.pOy;
        $('#pOz').value = jCmd.pOz;
        $('#gSize').value = jCmd.gSize;
        $('#cSize').value = jCmd.cSize;
    }
}
