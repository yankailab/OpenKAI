window.onload = function () {
    wsInit();

    // controls
    $('#btnForward').onmousedown = function (e) {
        cmdSetSteerSpeed(0,1);
    };

    $('#btnForward').onmouseup = function (e) {
        cmdSetSteerSpeed(0,0);
    };

    $('#btnBack').onmousedown = function (e) {
        cmdSetSteerSpeed(0,-1);
    };

    $('#btnBack').onmouseup = function (e) {
        cmdSetSteerSpeed(0,0);
    };

    $('#btnLeft').onmousedown = function (e) {
        cmdSetSteerSpeed(-1,0);
    };

    $('#btnLeft').onmouseup = function (e) {
        cmdSetSteerSpeed(0,0);
    };

    $('#btnRight').onmousedown = function (e) {
        cmdSetSteerSpeed(1,0);
    };

    $('#btnRight').onmouseup = function (e) {
        cmdSetSteerSpeed(0,0);
    };

    // $('#btnStop').onmousedown = function (e) {
    //     cmdSetSteerSpeed(0,0);
    // };

    function cmdSetSteerSpeed(setSteer, setSpeed) {
        var cmd = {
            cmd: 'setSteerSpeed',
            module: 'apDrive',
            steer: setSteer,
            speed: setSpeed,
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };

    $('#btnArm').onclick = function (e) {
        cmdSetArm(true);
    };

    $('#btnDisarm').onclick = function (e) {
        cmdSetArm(false);
    };

    function cmdSetArm(bSetArm) {
        var cmd = {
            cmd: 'setArm',
            module: 'apBase',
            bArm: bSetArm,
        };

        cmdStr = JSON.stringify(cmd) + strEOJ;
        wsSocket.send(cmdStr);
    };
};


function cmdHandler(event) {

    $('#txtMsg').value = event.data + "\n\n" + $('#txtMsg').value;

    jCmd = JSON.parse(event.data);
    if (jCmd.cmd == 'update')
    {

    }
};
