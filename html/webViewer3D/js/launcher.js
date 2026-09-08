/* Classic script intentionally works with file://. ES modules load only over HTTP. */
(() => {
  const host = document.querySelector('#host');
  const port = document.querySelector('#port');
  if (location.hostname) host.value = location.hostname.replace(/^\[|\]$/g, '');
  if (location.port) port.value = location.port;
  window.viewerEndpoint = () => {
    let hostname = host.value.trim().replace(/^\[|\]$/g, '');
    const number = Number(port.value);
    if (!hostname || /[\s/@?#\\]/.test(hostname) || !Number.isInteger(number) || number < 1 || number > 65535)
      throw new Error('Enter a valid host and port (1–65535).');
    if (hostname.includes(':')) hostname = `[${hostname}]`;
    return new URL(`${location.protocol === 'https:' ? 'https:' : 'http:'}//${hostname}:${number}/`);
  };
  if (location.protocol === 'file:') {
    document.querySelector('#connection').addEventListener('submit', event => {
      event.preventDefault();
      try { location.href = `${window.viewerEndpoint()}#connect`; }
      catch (error) { document.querySelector('#status').textContent = error.message; }
    });
  } else {
    document.querySelector('#start').disabled = true;
    import('./main.js').catch(error => {
      document.querySelector('#status').textContent = `Viewer could not load: ${error.message}`;
    });
  }
})();
