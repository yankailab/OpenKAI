window.onload = function() {
  function $(selector) {
      return document.querySelector(selector);
  }
      
  var socket = new WebSocket('ws://127.0.0.1:7890');
  socket.onopen = function(event)
  {
      $('#msg').innerHTML = 'Connected';
  };
  socket.onmessage = function(event)
  {
      $('#msg').innerHTML = 'Received';        
      $('#msgRecv').innerHTML = event.data;
  };
  socket.onclose = function(event)
  {
      $('#msg').innerHTML = 'Disconnected: ' + event.reason;
  };

  $('#submit').onclick = function(e)
  {
      socket.send($('#message').value);
      $('#msgSend').innerHTML = 'Sent:<br>' + $('input').value + '<br>';
  };

};

