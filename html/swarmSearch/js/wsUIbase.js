function wsInit()
{
//    function $(selector) { return document.querySelector(selector);}
  
  wsSocket = new WebSocket('ws://127.0.0.1:7890');

  wsSocket.onopen = function(event)
  {
      $('#state').innerHTML = 'Connected';
  };

  wsSocket.onmessage = function(event)
  {
      jc = JSON.parse(event.data);
      cmdHandler(jc);
  };

  wsSocket.onclose = function(event)
  {
      $('#state').innerHTML = 'Disconnected: ' + event.reason;
  };

//   $('#submit').onclick = function(e)
//   {
//       socket.send($('#message').value);
//       $('#msgSend').innerHTML = 'Sent:<br>' + $('input').value + '<br>';
//   };

};

