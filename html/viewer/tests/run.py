#!/usr/bin/env python3
"""Run the real viewer decoders against fixtures emitted by viewer_snapshot_test."""
import argparse
import functools
import http.server
import pathlib
import subprocess
import tempfile
import threading


class Handler(http.server.SimpleHTTPRequestHandler):
    fixture_directory = None

    def translate_path(self, path):
        if path.startswith('/fixtures/'):
            return str(self.fixture_directory / pathlib.PurePosixPath(path).name)
        return super().translate_path(path)

    def log_message(self, *args):
        pass


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('fixture_executable', type=pathlib.Path)
    parser.add_argument('--chrome', default='/opt/google/chrome/chrome')
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[3]
    with tempfile.TemporaryDirectory(prefix='openkai-viewer-test-') as temporary:
        temporary = pathlib.Path(temporary)
        Handler.fixture_directory = temporary
        subprocess.run([str(args.fixture_executable.resolve()), str(temporary)], check=True)
        server = http.server.ThreadingHTTPServer(('127.0.0.1', 0),
            functools.partial(Handler, directory=str(root)))
        worker = threading.Thread(target=server.serve_forever, daemon=True)
        worker.start()
        try:
            result = subprocess.run([
                args.chrome, '--headless', '--no-sandbox', '--disable-gpu',
                '--no-proxy-server', '--disable-background-networking',
                f'--user-data-dir={temporary / "chrome"}', '--dump-dom',
                '--virtual-time-budget=10000',
                f'http://127.0.0.1:{server.server_port}/html/viewer/tests/snapshots.html',
            ], text=True, capture_output=True, timeout=40)
            if result.returncode or '<pre id="result">PASS</pre>' not in result.stdout:
                raise RuntimeError(result.stdout + '\n' + result.stderr)
            print('PASS: C++/JavaScript point and line snapshots, empty clears, protocol rejection')
        finally:
            server.shutdown()
            server.server_close()
            worker.join()


if __name__ == '__main__':
    main()
