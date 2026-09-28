# Viewer snapshot checks

Build the standalone DataStream tests and run the browser test with Chrome:

```sh
cmake -S test/DataStream -B /tmp/openkai-stream-tests
cmake --build /tmp/openkai-stream-tests
ctest --test-dir /tmp/openkai-stream-tests --output-on-failure
python3 html/viewer/tests/run.py /tmp/openkai-stream-tests/viewer_snapshot_test
```

Use `--chrome /path/to/chrome` if Chrome is installed elsewhere. The runner binds
only a temporary localhost HTTP server and writes fixtures and browser state to
a temporary directory.

The C++ checks cover immutable snapshot cache identity, equal timestamp updates,
empty publications, filtering, rendering caps, and expiry boundaries. The
browser checks decode the actual C++ point/line bytes, reject other protocol
versions, and run both viewers' update paths to confirm that an empty point
publication clears points while preserving lines, and vice versa.
