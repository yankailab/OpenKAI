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


    $('#pOxInc').onclick = function (e) {
        $('#pOx').value = parseFloat($('#pOx').value) + dV;
        cmdSetGrid();
    };

    $('#pOxDec').onclick = function (e) {
        $('#pOx').value = parseFloat($('#pOx').value) - dV;
        cmdSetGrid();
    };

    $('#pOyInc').onclick = function (e) {
        $('#pOy').value = parseFloat($('#pOy').value) + dV;
        cmdSetGrid();
    };

    $('#pOyDec').onclick = function (e) {
        $('#pOy').value = parseFloat($('#pOy').value) - dV;
        cmdSetGrid();
    };

    $('#pOzInc').onclick = function (e) {
        $('#pOz').value = parseFloat($('#pOz').value) + dV;
        cmdSetGrid();
    };

    $('#pOzDec').onclick = function (e) {
        $('#pOz').value = parseFloat($('#pOz').value) - dV;
        cmdSetGrid();
    };



    $('#vGxInc').onclick = function (e) {
        $('#vGx').value = parseFloat($('#vGx').value) + 1;
        cmdSetGrid();
    };

    $('#vGxDec').onclick = function (e) {
        $('#vGx').value = parseFloat($('#vGx').value) - 1;
        cmdSetGrid();
    };

    $('#vGyInc').onclick = function (e) {
        $('#vGy').value = parseFloat($('#vGy').value) + 1;
        cmdSetGrid();
    };

    $('#vGyDec').onclick = function (e) {
        $('#vGy').value = parseFloat($('#vGy').value) - 1;
        cmdSetGrid();
    };

    $('#vGzInc').onclick = function (e) {
        $('#vGz').value = parseFloat($('#vGz').value) + 1;
        cmdSetGrid();
    };

    $('#vGzDec').onclick = function (e) {
        $('#vGz').value = parseFloat($('#vGz').value) - 1;
        cmdSetGrid();
    };



    $('#vCellSizeXinc').onclick = function (e) {
        $('#vCellSizeX').value = parseFloat($('#vCellSizeX').value) + dV;
        cmdSetGrid();
    };

    $('#vCellSizeXdec').onclick = function (e) {
        $('#vCellSizeX').value = parseFloat($('#vCellSizeX').value) - dV;
        cmdSetGrid();
    };

    $('#vCellSizeYinc').onclick = function (e) {
        $('#vCellSizeY').value = parseFloat($('#vCellSizeY').value) + dV;
        cmdSetGrid();
    };

    $('#vCellSizeYdec').onclick = function (e) {
        $('#vCellSizeY').value = parseFloat($('#vCellSizeY').value) - dV;
        cmdSetGrid();
    };

    $('#vCellSizeZinc').onclick = function (e) {
        $('#vCellSizeZ').value = parseFloat($('#vCellSizeZ').value) + dV;
        cmdSetGrid();
    };

    $('#vCellSizeZdec').onclick = function (e) {
        $('#vCellSizeZ').value = parseFloat($('#vCellSizeZ').value) - dV;
        cmdSetGrid();
    };


    function cmdSetGrid()
    {
        var cmd = {
            cmd: 'setGrid',
            pOx: parseFloat($('#pOx').value),
            pOy: parseFloat($('#pOy').value),
            pOz: parseFloat($('#pOz').value),

            vGx: parseFloat($('#vGx').value),
            vGy: parseFloat($('#vGy').value),
            vGz: parseFloat($('#vGz').value),

            vCx: parseFloat($('#vCellSizeX').value),
            vCy: parseFloat($('#vCellSizeY').value),
            vCz: parseFloat($('#vCellSizeZ').value),
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnSetGrid').onclick = function (e) {
        cmdSetGrid();
    };

    $('#btnSaveGrid').onclick = function (e) {
        var cmd = {
            cmd: 'saveGrid',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnAutoAlertCells').onclick = function (e) {
        autoAlertCells();
    };

    function autoAlertCells()
    {
        var cmd = {
            cmd: 'autoAlertCells',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    }


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
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTxAll').value = dV;
        cmdSetTRall();
    };

    $('#vTxAllDec').onclick = function (e) {
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTxAll').value = -dV;
        cmdSetTRall();
    };

    $('#vTyAllInc').onclick = function (e) {
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTyAll').value = dV;
        cmdSetTRall();
    };

    $('#vTyAllDec').onclick = function (e) {
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTyAll').value = -dV;
        cmdSetTRall();
    };

    $('#vTzAllInc').onclick = function (e) {
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTzAll').value = dV;
        cmdSetTRall();
    };

    $('#vTzAllDec').onclick = function (e) {
        $('#vTxAll').value = 0;
        $('#vTyAll').value = 0;
        $('#vTzAll').value = 0;

        $('#vTzAll').value = -dV;
        cmdSetTRall();
    };


    // R
    $('#vRxAllInc').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRxAll').value = dV;
        cmdSetTRall();
    };

    $('#vRxAllDec').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRxAll').value = -dV;
        cmdSetTRall();
    };

    $('#vRyAllInc').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRyAll').value = dV;
        cmdSetTRall();
    };

    $('#vRyAllDec').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRyAll').value = -dV;
        cmdSetTRall();
    };

    $('#vRzAllInc').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRzAll').value = dV;
        cmdSetTRall();
    };

    $('#vRzAllDec').onclick = function (e) {
        $('#vRxAll').value = 0;
        $('#vRyAll').value = 0;
        $('#vRzAll').value = 0;

        $('#vRzAll').value = -dV;
        cmdSetTRall();
    };

    function cmdSetTRall() {
        var cmd = {
            cmd: 'setTRall',
            vTx: parseFloat($('#vTxAll').value),
            vTy: parseFloat($('#vTyAll').value),
            vTz: parseFloat($('#vTzAll').value),
            vRx: parseFloat($('#vRxAll').value),
            vRy: parseFloat($('#vRyAll').value),
            vRz: parseFloat($('#vRzAll').value),
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
    if (jCmd.cmd == "updateTR") {
        $('#_GeometryBase').value = jCmd._GeometryBase;
        $('#vTx').value = jCmd.vTx;
        $('#vTy').value = jCmd.vTy;
        $('#vTz').value = jCmd.vTz;
        $('#vRx').value = jCmd.vRx;
        $('#vRy').value = jCmd.vRy;
        $('#vRz').value = jCmd.vRz;
    }
    else if (jCmd.cmd == "updateGrid") {
        $('#pOx').value = jCmd.pOx;
        $('#pOy').value = jCmd.pOy;
        $('#pOz').value = jCmd.pOz;
        $('#vGx').value = jCmd.vGx;
        $('#vGy').value = jCmd.vGy;
        $('#vGz').value = jCmd.vGz;
        $('#vCellSizeX').value = jCmd.vCx;
        $('#vCellSizeY').value = jCmd.vCy;
        $('#vCellSizeZ').value = jCmd.vCz;
    }
}
