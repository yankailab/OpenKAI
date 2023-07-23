window.onload = function() {
  function $(selector) {
      return document.querySelector(selector);
  }
      
  var strEOJ = "EOJ";
  
  $('#heartbeat').onclick = function(e)
  {
      cmd = {
          cmd:'heartbeat',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };
  
  $('#takeoffRequest').onclick = function(e)
  {
      cmd = {
          cmd:'takeoffRequest',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#takeoffStatus').onclick = function(e)
  {
      cmd = {
          cmd:'takeoffStatus',
          t:12345,
          id:1,
          stat:'airborne',
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#landingRequest').onclick = function(e)
  {
      cmd = {
          cmd:'landingRequest',
          t:12345,
          id:1,
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#landingStatus').onclick = function(e)
  {
      cmd = {
          cmd:'landingStatus',
          t:12345,
          id:1,
          stat:'standby',
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#ackLandingRequest').onclick = function(e)
  {
      cmd = {
          cmd:'ackLandingRequest',
          t:12345,
          id:1,
          result:'ok',
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

  $('#ackTakeoffRequest').onclick = function(e)
  {
      cmd = {
          cmd:'ackTakeoffRequest',
          t:12345,
          id:1,
          result:'ok',
      };
          
      cmdStr = JSON.stringify(cmd) + strEOJ;
      socket.send(cmdStr);
      $('#msgSend').innerHTML = cmdStr;
  };

};

