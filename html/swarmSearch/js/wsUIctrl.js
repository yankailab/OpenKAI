window.onload = function()
{
    function $(selector) { return document.querySelector(selector);}

    wsInit();

  var strEOJ = "EOJ";
  
  $('#btnStandby').onclick = function(e)
  {
      cmd = {
          cmd:'standby',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
      $('#state').innerHTML = cmdStr;
  };
  
  $('#btnTakeoff').onclick = function(e)
  {
      cmd = {
          cmd:'takeoff',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#btnAuto').onclick = function(e)
  {
      cmd = {
          cmd:'auto',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#btnRTL').onclick = function(e)
  {
      cmd = {
          cmd:'rtl',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

};

function cmdHandler(jCmd)
{


}


