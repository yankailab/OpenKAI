function $(selector) { return document.querySelector(selector); }

var strEOJ = "EOJ";

window.onload = function () {
    wsInit();

    // controls
    $('#btnUpdate').onclick = function (e) {
        cmdLCalign('update');
    };

    $('#btnLoadCfg').onclick = function (e) {
        cmdLCalign('loadCfg');
    };

    $('#btnSaveCfg').onclick = function (e) {
        cmdLCalign('saveCfg');
    };

    $('#btnSavePly').onclick = function (e) {
        cmdLCalign('savePly');
    };

    function cmdLCalign(c) {
        var cmd = {
            cmd: c,
            module: $('#mName').value,
            fNameCfg: $('#fNameCfg').value,
            fNamePly: $('#dataDir').value + getTimestamp() + "_tLow" + $('#tempLow').value + "_tHigh" + $('#tempHigh').value + '.ply',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


    $('#btnSaveImg').onclick = function (e) {
        cmdThermal('saveImg');
    };

    function cmdThermal(c) {
        var cmd = {
            cmd: c,
            module: $('#mThermal').value,
            fNameImg: $('#dataDir').value + getTimestamp() + "_tLow" + $('#tempLow').value + "_tHigh" + $('#tempHigh').value + '.png',
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


    // param set
    $('#Cfx').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cfy').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ccx').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ccy').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ck1').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ck2').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cp1').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cp2').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cp3').oninput = function (e) {
        cmdSetParams();
    };

    $('#Crx').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cry').oninput = function (e) {
        cmdSetParams();
    };

    $('#Crz').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ctx').oninput = function (e) {
        cmdSetParams();
    };

    $('#Cty').oninput = function (e) {
        cmdSetParams();
    };

    $('#Ctz').oninput = function (e) {
        cmdSetParams();
    };

    function cmdSetParams() {
        var cmd = {
            cmd: 'setParams',
            module: $('#mName').value,
            vCf: [parseFloat($('#Cfx').value), parseFloat($('#Cfy').value)],
            vCc: [parseFloat($('#Ccx').value), parseFloat($('#Ccy').value)],
            aCdist: [parseFloat($('#Ck1').value), parseFloat($('#Ck2').value), parseFloat($('#Cp1').value), parseFloat($('#Cp2').value), parseFloat($('#Cp3').value)],
            vCr: [parseFloat($('#Crx').value), parseFloat($('#Cry').value), parseFloat($('#Crz').value)],
            aCt: [parseFloat($('#Ctx').value), parseFloat($('#Cty').value), parseFloat($('#Ctz').value)],
        };
        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };


    $('#tempLow').oninput = function (e) {
        cmdSetThermal();
    };

    $('#tempHigh').oninput = function (e) {
        cmdSetThermal();
    };

    function cmdSetThermal() {
        var cmd = {
            cmd: 'setThermal',
            module: $('#mThermal').value,
            vTrange: [parseFloat($('#tempLow').value), parseFloat($('#tempHigh').value)],
        };
        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

};


function cmdHandler(event) {

    $('#txtMsg').value = event.data + "\n\n" + $('#txtMsg').value;

    jCmd = JSON.parse(event.data);

    if (jCmd.cmd == 'update') {
        $('#Cfx').value = jCmd.vCf[0];
        $('#Cfy').value = jCmd.vCf[1];

        $('#Ccx').value = jCmd.vCc[0];
        $('#Ccy').value = jCmd.vCc[1];

        $('#Ck1').value = jCmd.aCdist[0];
        $('#Ck2').value = jCmd.aCdist[1];
        $('#Cp1').value = jCmd.aCdist[2];
        $('#Cp2').value = jCmd.aCdist[3];
        $('#Cp3').value = jCmd.aCdist[4];

        // $('#Crx').value
        // $('#Cry').value
        // $('#Crz').value

        $('#Ctx').value = jCmd.aCt[0];
        $('#Cty').value = jCmd.aCt[1];
        $('#Ctz').value = jCmd.aCt[2];
    }
};

function getTimestamp() {
    const d = new Date();
    const yyyy = d.getFullYear();
    const mm = String(d.getMonth() + 1).padStart(2, '0');
    const dd = String(d.getDate()).padStart(2, '0');
    const hh = String(d.getHours()).padStart(2, '0');
    const mi = String(d.getMinutes()).padStart(2, '0');
    const ss = String(d.getSeconds()).padStart(2, '0');
    return `${yyyy}-${mm}-${dd}_${hh}-${mi}-${ss}`;
};
