/* Classic script intentionally works with file://. ES modules load only over HTTP. */
(() => {
  const host = document.querySelector('#host');
  const port = document.querySelector('#port');
  const cmdPort = document.querySelector('#cmdPort');
  if (location.hostname) host.value = location.hostname.replace(/^\[|\]$/g, '');
  if (location.port) port.value = location.port;
  const savedCmdPort = new URLSearchParams(location.search).get('cmdPort');
  if (savedCmdPort !== null) cmdPort.value = savedCmdPort;
  window.viewerEndpoint = (portValue = port.value) => {
    let hostname = host.value.trim().replace(/^\[|\]$/g, '');
    const number = Number(portValue);
    if (!hostname || /[\s/@?#\\]/.test(hostname) || !Number.isInteger(number) || number < 1 || number > 65535)
      throw new Error('Enter a valid host and port (1–65535).');
    if (hostname.includes(':')) hostname = `[${hostname}]`;
    return new URL(`${location.protocol === 'https:' ? 'https:' : 'http:'}//${hostname}:${number}/`);
  };
  if (location.protocol === 'file:') {
    document.querySelector('#connection').addEventListener('submit', event => {
      event.preventDefault();
      try {
        const endpoint = window.viewerEndpoint();
        // Carry the command port through the file:// -> HTTP launcher navigation.
        endpoint.searchParams.set('cmdPort', cmdPort.value);
        endpoint.hash = 'connect';
        location.href = endpoint.href;
      }
      catch (error) { document.querySelector('#status').textContent = error.message; }
    });
  } else {
    document.querySelector('#start').disabled = true;
    import('./main.js').catch(error => {
      document.querySelector('#status').textContent = `Viewer could not load: ${error.message}`;
    });
  }
})();
