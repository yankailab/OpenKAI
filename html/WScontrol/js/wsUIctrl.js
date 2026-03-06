function $(selector) { return document.querySelector(selector); }

var strEOJ = "EOJ";

window.onload = function () {
    wsInit();

    // controls
    $('#btnUpdate').onclick = function (e) {
        cmdLIVcalib('update');
    };

    $('#btnLoadCfg').onclick = function (e) {
        cmdLIVcalib('loadCfg');
    };

    $('#btnSaveCfg').onclick = function (e) {
        cmdLIVcalib('saveCfg');
    };

    $('#btnSavePly').onclick = function (e) {
        cmdLIVcalib('savePly');
    };

    function cmdLIVcalib(c) {
        var cmd = {
            cmd: c,
            module: $('#mName').value,
            fNameCfg : $('#fNameCfg').value,
            fNamePly : $('#fNamePly').value,
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
            aCdistortion: [parseFloat($('#Ck1').value), parseFloat($('#Ck2').value), parseFloat($('#Cp1').value), parseFloat($('#Cp2').value), parseFloat($('#Cp3').value)],
            vCr: [parseFloat($('#Crx').value), parseFloat($('#Cry').value), parseFloat($('#Crz').value)],
            aCt: [parseFloat($('#Ctx').value), parseFloat($('#Cty').value), parseFloat($('#Ctz').value)],
        };
        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

};


function cmdHandler(event) {

    $('#txtMsg').value = event.data + "\n\n" + $('#txtMsg').value;

    jCmd = JSON.parse(event.data);
}
