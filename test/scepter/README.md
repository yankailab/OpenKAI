`_Scepter` accepts `pclStride` in its camera configuration:

| Value | Sample spacing | Approximate point density |
| --- | --- | --- |
| `1` (default) | Every pixel | Full |
| `2` | Every second row and column | 1/4 |
| `3` | Every third row and column | 1/9 |
| `4` | Every fourth row and column | 1/16 |

`jsonCfg/Scepter.json` enables `pclStride: 2`. Larger strides trade spatial
detail for faster point-cloud publication. Values must be integers from 1
through 65535. Sampling starts at pixel (0, 0) and skips invalid depths.

Sampling applies to both colored and uncolored point clouds in the Scepter
driver. SDK conversion, calibration, and RGB/depth previews retain their native
resolution; each selected point keeps the color from its original aligned pixel.

After building OpenKAI with Scepter and universe support, run the hardware-free
regression checks with `python3 test/scepter/native.py build`. They cover frame
validation, depth previews, capture synchronization, stride configuration,
odd-sized frames, color correspondence, and strides larger than the image.

## Web viewer and configuration

```sh
python3 test/scepter/config.py build
python3 test/scepter/browser.py build/OpenKAI
```

The config check simulates SDK setters to cover full snapshots, partial-file
loading, validation, mode dependencies, rejected indexed values and capture
restart. It does not need a camera. The browser check uses Chrome/Chromium and
localhost sockets with temporary ports and files, starting from
`jsonCfg/Scepter.json`. It checks WSconsole controls and persistence, and live points
when a camera opens. See [the viewer README](../../html/viewer/_Scepter/README.md).
