/* file:// cannot load ES modules; open the embedded HTTP viewer instead. */
(() => {
  const host = document.querySelector('#host');
  const port = document.querySelector('#port');
  if (location.hostname) host.value = location.hostname.replace(/^\[|\]$/g, '');
  port.value = location.port || (location.protocol === 'https:' ? '443' : '80');
  if (location.protocol === 'file:') port.value = '8080';
  window.viewerEndpoint = () => {
    let hostname = host.value.trim().replace(/^\[|\]$/g, '');
    const number = Number(port.value);
    if (!hostname || /[\s/@?#\\]/.test(hostname) || !Number.isInteger(number) || number < 1 || number > 65535)
      throw new Error('Enter a valid host and port (1–65535).');
    if (hostname.includes(':')) hostname = `[${hostname}]`;
    return new URL(`${location.protocol === 'https:' ? 'https:' : 'http:'}//${hostname}:${number}/`);
  };
  if (location.protocol === 'file:') {
    document.querySelector('#start').disabled = false;
    document.querySelector('#status').textContent = 'Open through the embedded HTTP server';
    document.querySelector('#connection').addEventListener('submit', event => {
      event.preventDefault();
      try { const endpoint = window.viewerEndpoint(); endpoint.hash = 'connect'; location.href = endpoint.href; }
      catch (error) { document.querySelector('#status').textContent = error.message; }
    });
  } else {
    import('./main.js').catch(error => {
      document.querySelector('#status').textContent = `Viewer could not load: ${error.message}`;
      document.querySelector('#status').dataset.state = 'error';
    });
  }
})();
