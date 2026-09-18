/* Page-specific handlers go here. The socket lifecycle lives in wsCmdBase.js.
 * Example: button.addEventListener('click', () =>
 *   wsSendCmd({ cmd: 'test', module: 'tester', v: 0 }));
 */
var wsCmdBuffer = '';
var wsCmdScan = 0, wsCmdDepth = 0, wsCmdQuoted = false, wsCmdEscaped = false;
const WS_CMD_MAX_REPLY = 8 * 1024 * 1024;

function cmdResetFrame() { wsCmdScan = wsCmdDepth = 0; wsCmdQuoted = wsCmdEscaped = false; }
function cmdReset() { wsCmdBuffer = ''; cmdResetFrame(); }

function cmdHandler(event) {
  if (typeof event.data !== 'string') {
    wsCmdLog('Ignored non-text command message');
    return;
  }
  wsCmdBuffer += event.data;
  if (wsCmdBuffer.length > WS_CMD_MAX_REPLY) {
    cmdReset();
    wsCmdLog('Command reply exceeds 8 MiB');
    return;
  }

  // _WebSocketServer can split a JSON reply into 512-byte text messages.
  // Find complete objects without treating braces/EOJ inside JSON strings as framing.
  while (wsCmdBuffer.length) {
    if (!wsCmdScan) {
      wsCmdBuffer = wsCmdBuffer.trimStart();
      if (!wsCmdBuffer || (wsCmdBuffer.length < strEOJ.length && strEOJ.startsWith(wsCmdBuffer))) return;
      if (wsCmdBuffer.startsWith(strEOJ)) {
        wsCmdBuffer = wsCmdBuffer.slice(strEOJ.length);
        continue;
      }
      if (wsCmdBuffer[0] !== '{') {
        wsCmdLog('Invalid JSON reply: ' + wsCmdBuffer.slice(0, 160));
        cmdReset();
        return;
      }
    }
    // Retain scan state across fragments so large cell lists are scanned once.
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
      const jCmd = JSON.parse(text);
      wsCmdLog(text);
      handleCmd(jCmd);
    } catch (error) { wsCmdLog('Command reply error: ' + error.message); }
  }
}

function handleCmd(jCmd) {
  if (jCmd.cmd === 'setGridConfig') {
    window.dispatchEvent(new CustomEvent('gridconfig', { detail: jCmd }));
  }
  if (jCmd.cmd === 'cellSelect') {
    window.dispatchEvent(new CustomEvent('cellselect', { detail: jCmd }));
  }
  if (jCmd.cmd === 'ackTest') {
    // ackTest(jCmd);
  }
  // _WSconsole also sends { cmd: 'hb' }; add application reply handlers here.
}
