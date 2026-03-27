function $(selector) { return document.querySelector(selector); }

var strEOJ = "EOJ";

function wsInit()
{
  wsSocket = new WebSocket("ws://127.0.0.1:7890");
  //  wsSocket = new WebSocket(`ws://${location.hostname}:7890`);
  //    wsSocket = new WebSocket("ws://192.168.8.188:7890");

  wsSocket.onopen = function (event) {
    $('#state').innerHTML = 'Connected';
  };

  wsSocket.onmessage = function (event) {
    cmdHandler(event);
  };

  wsSocket.onclose = function (event) {
    $('#state').innerHTML = 'Disconnected: ' + event.reason;
  };

};
