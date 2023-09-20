function $(selector) { return document.querySelector(selector);}

window.onload = function()
{
    wsInit();

  var strEOJ = "EOJ";
  var dV = 1;

  $('#spdInc').onclick = function(e)
  {
      $('#spd').value = parseFloat($('#spd').value) + dV;
  };        

  $('#spdDec').onclick = function(e)
  {
      $('#spd').value = parseFloat($('#spd').value) - dV;
  };        

  $('#dirL').onclick = function(e)
  {
      $('#direction').value = 1.0;
  };        

  $('#dirR').onclick = function(e)
  {
      $('#direction').value = -1.0;
  };        

  $('#distInc').onclick = function(e)
  {
      $('#dist').value = parseFloat($('#dist').value) + dV;
  };        

  $('#distDec').onclick = function(e)
  {
      $('#dist').value = parseFloat($('#dist').value) - dV;
  };

  $('#dIntInc').onclick = function(e)
  {
      $('#dInt').value = parseFloat($('#dInt').value) + dV;
  };        

  $('#dIntDec').onclick = function(e)
  {
      $('#dInt').value = parseFloat($('#dInt').value) - dV;
  };        
  
  

  $('#btnReset').onclick = function(e)
  {
      var cmd = {
        cmd:'reset',
        id:1,
      };

      cmdStr = JSON.stringify(cmd) + strEOJ;
      wsSocket.send(cmdStr);
  };
  
  $('#btnMove').onclick = function(e)
  {
      var cmd = {
          cmd:'move',
          spd:parseFloat($('#spd').value),
          direction:parseFloat($('#direction').value),
          dist:parseFloat($('#dist').value),
          dInt:parseFloat($('#dInt').value),
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

  $('#btnTake').onclick = function(e)
  {
      var cmd = {
          cmd:'take',
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

};

function cmdHandler(jCmd)
{
    if(jCmd.cmd == "hb")
    {
      $('#state').innerHTML = jCmd.s.toUpperCase();
      $('#msg').innerHTML = jCmd.msg;
    }
}
