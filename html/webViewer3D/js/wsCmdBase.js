/* Classic-script API for page-specific command controls; no renderer dependency. */
function $(selector) { return document.querySelector(selector); }

var strEOJ = 'EOJ';
var wsSocket = null;
var wsCmdTimer = null;

function wsCmdLog(message) {
  const console = $('#cmdState');
  if (console) console.value = (String(message) + '\n\n' + console.value).slice(0, 16384);
}

function wsCmdActive() {
  return wsSocket !== null && wsSocket.readyState < WebSocket.CLOSING;
}

function wsCmdState(message) {
  wsCmdLog(message);
  $('#cmdConnect').disabled = wsCmdActive();
  $('#cmdDisconnect').disabled = !wsCmdActive();
  window.dispatchEvent(new Event('wscmdstatechange'));
}

function wsInit(endpoint) {
  try {
    const url = new URL(endpoint || window.viewerEndpoint($('#cmdPort').value));
    url.protocol = url.protocol === 'https:' || url.protocol === 'wss:' ? 'wss:' : 'ws:';
    url.search = url.hash = '';
    // Keep an existing connection when Start is clicked again for the same server.
    if (wsCmdActive() && wsSocket.url === url.href) return true;
    wsStop(false);
    const socket = wsSocket = new WebSocket(url);
    wsCmdState(`Connecting to ${url.host}…`);
    wsCmdTimer = setTimeout(() => {
      if (wsSocket === socket && socket.readyState === WebSocket.CONNECTING) {
        wsStop(false);
        wsCmdState('Command connection timed out');
      }
    }, 10000);
    socket.onopen = function () {
      if (wsSocket !== socket) return;
      clearTimeout(wsCmdTimer);
      wsCmdState(`Connected to ${url.host}`);
    };
    socket.onmessage = function (event) {
      if (wsSocket === socket) cmdHandler(event);
    };
    socket.onclose = function (event) {
      if (wsSocket !== socket) return;
      clearTimeout(wsCmdTimer);
      wsSocket = null;
      cmdReset();
      wsCmdState(`Disconnected (${event.code})${event.reason ? ': ' + event.reason : ''}`);
    };
    socket.onerror = function () {
      if (wsSocket === socket) wsCmdState(`Command connection failed: ${url.host}`);
    };
    return true;
  } catch (error) {
    wsCmdState(error.message);
    return false;
  }
}

function wsStop(log = true) {
  clearTimeout(wsCmdTimer);
  const socket = wsSocket;
  wsSocket = null; // Ignore late events from an old connection.
  if (socket) socket.close();
  cmdReset();
  if (log) wsCmdState('Disconnected');
}

function wsSendCmd(command) {
  if (!wsSocket || wsSocket.readyState !== WebSocket.OPEN) {
    wsCmdLog('Command not sent: disconnected');
    return false;
  }
  try {
    if (!command || typeof command !== 'object' || Array.isArray(command)) throw new Error('Command must be a JSON object');
    const text = JSON.stringify(command);
    // _JSONbase::recvJson uses a literal terminator, including inside strings.
    if (text.includes(strEOJ)) throw new Error('Command contains the reserved EOJ delimiter');
    wsSocket.send(text + strEOJ);
    return true;
  } catch (error) {
    wsCmdLog(`Command not sent: ${error.message}`);
    return false;
  }
}

$('#cmdConnect').addEventListener('click', () => wsInit());
$('#cmdDisconnect').addEventListener('click', () => wsStop());
window.addEventListener('pagehide', () => wsStop(false));
