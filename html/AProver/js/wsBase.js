function $(selector) { return document.querySelector(selector); }

var strEOJ = "EOJ";
var wsSocket = null;
var wsCmdBuffer = '';
var wsCmdScan = 0, wsCmdDepth = 0, wsCmdQuoted = false, wsCmdEscaped = false;

function cmdResetFrame() { wsCmdScan = wsCmdDepth = 0; wsCmdQuoted = wsCmdEscaped = false; }
function cmdReset() { wsCmdBuffer = ''; cmdResetFrame(); }

function cmdHandler(event) {
  if (typeof event.data !== 'string') return;
  wsCmdBuffer += event.data;
  if (wsCmdBuffer.length > 8 * 1024 * 1024) {
    cmdReset();
    $('#state').textContent = 'Command reply too large';
    return;
  }

  // Match the shared viewers: replies may span multiple WebSocket messages,
  // contain several objects, or include EOJ terminators.
  while (wsCmdBuffer.length) {
    if (!wsCmdScan) {
      wsCmdBuffer = wsCmdBuffer.trimStart();
      if (!wsCmdBuffer || (wsCmdBuffer.length < strEOJ.length && strEOJ.startsWith(wsCmdBuffer))) return;
      if (wsCmdBuffer.startsWith(strEOJ)) {
        wsCmdBuffer = wsCmdBuffer.slice(strEOJ.length);
        continue;
      }
      if (wsCmdBuffer[0] !== '{') { cmdReset(); return; }
    }
    let end = 0;
    while (wsCmdScan < wsCmdBuffer.length) {
      const c = wsCmdBuffer[wsCmdScan++];
      if (wsCmdQuoted) {
        if (wsCmdEscaped) wsCmdEscaped = false;
        else if (c === '\\') wsCmdEscaped = true;
        else if (c === '"') wsCmdQuoted = false;
      } else if (c === '"') wsCmdQuoted = true;
      else if (c === '{') ++wsCmdDepth;
      else if (c === '}' && --wsCmdDepth === 0) { end = wsCmdScan; break; }
    }
    if (!end) return;
    const text = wsCmdBuffer.slice(0, end);
    wsCmdBuffer = wsCmdBuffer.slice(end);
    cmdResetFrame();
    try {
      const reply = JSON.parse(text);
      if (reply.cmd !== 'hb') $('#cmdState').value = (text + '\n\n' + $('#cmdState').value).slice(0, 12000);
      handleCmd(reply);
    } catch (error) { $('#state').textContent = 'Command reply error: ' + error.message; }
  }
}

function wsInit()
{
    cmdReset();
    const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
    wsSocket = new WebSocket(`${protocol}//${location.hostname || 'localhost'}:7890`);

  wsSocket.onopen = function (event) {
    $('#state').textContent = 'Connected';
  };

  wsSocket.onmessage = function (event) {
    cmdHandler(event);
  };

  wsSocket.onclose = function (event) {
    cmdReset();
    $('#state').textContent = 'Disconnected: ' + event.reason;
  };

  wsSocket.onerror = function (event) {
    $('#state').textContent = 'Connection error';
  };

};
