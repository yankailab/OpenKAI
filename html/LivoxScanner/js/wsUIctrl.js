function $(selector) { return document.querySelector(selector);}

window.onload = function()
{
    wsInit();

  var strEOJ = "EOJ";
  var dV = 1;
  
  $('#btnReset').onclick = function(e)
  {
      var cmd = {
        cmd:'reset',
        id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };
  
  $('#btnStart').onclick = function(e)
  {
      var cmd = {
          cmd:'start',
          id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnStop').onclick = function(e)
  {
      var cmd = {
          cmd:'stop',
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnSave').onclick = function(e)
  {
      var cmd = {
          cmd:'save',
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnSetConfig').onclick = function(e)
  {
      var cmd = {
          cmd:'setConfig',
          hFrom:parseFloat($('#hFrom').value),
          hStep:parseFloat($('#hStep').value),
          hTo:parseFloat($('#hTo').value),
          vFrom:parseFloat($('#vFrom').value),
          vStep:parseFloat($('#vStep').value),
          vTo:parseFloat($('#vTo').value),
          Xoffset:parseFloat($('#Xoffset').value),
          Yoffset:parseFloat($('#Yoffset').value),
          Zoffset:parseFloat($('#Zoffset').value),
          id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

  $('#btnGetConfig').onclick = function(e)
  {
      var cmd = {
          cmd:'getConfig',
          id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };

};

function cmdHandler(jCmd)
{
    if(jCmd.cmd == "hb")
    {
      $('#state').innerHTML = jCmd.s.toUpperCase();
      $('#msg').innerHTML = jCmd.msg;
    }
    else if(jCmd.cmd == "getConfig")
    {
      $('#hFrom').value = parseFloat(jCmd.hFrom);
      $('#hStep').value = parseFloat(jCmd.hStep);
      $('#hTo').value = parseFloat(jCmd.hTo);
      $('#vFrom').value = parseFloat(jCmd.vFrom);
      $('#vStep').value = parseFloat(jCmd.vStep);
      $('#vTo').value = parseFloat(jCmd.vTo);
      $('#Xoffset').value = parseFloat(jCmd.Xoffset);
      $('#Yoffset').value = parseFloat(jCmd.Yoffset);
      $('#Zoffset').value = parseFloat(jCmd.Zoffset);
      $('#msg').innerHTML = jCmd.msg;
    }
}
