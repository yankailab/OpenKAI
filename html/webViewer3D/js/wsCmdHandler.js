/* Page-specific handlers go here. The socket lifecycle lives in wsCmdBase.js.
 * Example: button.addEventListener('click', () =>
 *   wsSendCmd({ cmd: 'test', module: 'tester', v: 0 }));
 */
var wsCmdBuffer = '';

function cmdReset() { wsCmdBuffer = ''; }

function cmdHandler(event) {
  if (typeof event.data !== 'string') {
    wsCmdLog('Ignored non-text command message');
    return;
  }
  wsCmdBuffer += event.data;
  if (wsCmdBuffer.length > 65536) {
    cmdReset();
    wsCmdLog('Command reply exceeds 64 KiB');
    return;
  }

  // _WebSocketServer can split a JSON reply into 512-byte text messages.
  // Find complete objects without treating braces/EOJ inside JSON strings as framing.
  while (wsCmdBuffer.length) {
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
    let depth = 0, quoted = false, escaped = false, end = 0;
    for (let i = 0; i < wsCmdBuffer.length; ++i) {
      const c = wsCmdBuffer[i];
      if (quoted) {
        if (escaped) escaped = false;
        else if (c === '\\') escaped = true;
        else if (c === '"') quoted = false;
      } else if (c === '"') quoted = true;
      else if (c === '{') ++depth;
      else if (c === '}' && --depth === 0) { end = i + 1; break; }
    }
    if (!end) return;
    const text = wsCmdBuffer.slice(0, end);
    wsCmdBuffer = wsCmdBuffer.slice(end);
    try {
      const jCmd = JSON.parse(text);
      wsCmdLog(text);
      handleCmd(jCmd);
    } catch (error) { wsCmdLog('Command reply error: ' + error.message); }
  }
}

function handleCmd(jCmd) {
  if (jCmd.cmd === 'ackTest') {
    // ackTest(jCmd);
  }
  // _WSconsole also sends { cmd: 'hb' }; add application reply handlers here.
}
