# YOLO26 monocular depth estimation

`_YOLO26depthEstONNX` runs an Ultralytics YOLO26 depth model with ONNX Runtime. It takes an `RGBframe`, publishes aligned `RGBDframe` depth as `CV_32FC1` in metres, and can publish a camera-space XYZ point cloud with optional RGB colors through `PCLframe`.

## Models and export

All five official depth checkpoints and their FP32 ONNX exports are stored in `/home/kai/dev/models/YOLO26/`:

| Scale | Checkpoint | ONNX model | ONNX size |
| --- | --- | --- | --- |
| Nano | `yolo26n-depth.pt` | `yolo26n-depth.onnx` | 19.8 MiB |
| Small | `yolo26s-depth.pt` | `yolo26s-depth.onnx` | 45.9 MiB |
| Medium | `yolo26m-depth.pt` | `yolo26m-depth.onnx` | 84.3 MiB |
| Large | `yolo26l-depth.pt` | `yolo26l-depth.onnx` | 101.1 MiB |
| X-large | `yolo26x-depth.pt` | `yolo26x-depth.onnx` | 213.0 MiB |

These exports use a fixed `1 × 3 × 768 × 768` float32 input, `1 × 1 × 768 × 768` float32 output, batch size 1, and ONNX opset 17. Each passed the ONNX checker and CPU ONNX Runtime inference with finite positive depth output. `depth-export-manifest.json` in the model directory records checksums, tensor shapes, and conversion versions. This checks model execution, not depth accuracy on a particular camera or scene.

To reproduce the exports in an isolated environment:

```bash
sudo apt install python3-venv
python3 -m venv ~/venv/yolo26-depth
source ~/venv/yolo26-depth/bin/activate
python -m pip install --upgrade pip
python -m pip install torch torchvision --index-url https://download.pytorch.org/whl/cpu
python -m pip install ultralytics==8.4.166 onnx onnxruntime onnxscript

mkdir -p /home/kai/dev/models/YOLO26
cd /home/kai/dev/models/YOLO26
for size in n s m l x; do
  curl --fail --location --retry 3 \
    "https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26${size}-depth.pt" \
    --output "yolo26${size}-depth.pt"
done

python - <<'PY'
from pathlib import Path
import numpy as np
import onnx
import onnxruntime as ort
from ultralytics import YOLO

directory = Path('/home/kai/dev/models/YOLO26')
for size in 'nsmlx':
    model = YOLO(str(directory / f'yolo26{size}-depth.pt'), task='depth')
    path = model.export(format='onnx', imgsz=768, batch=1, dynamic=False,
                        simplify=False, opset=17, device='cpu')
    onnx.checker.check_model(path)
    session = ort.InferenceSession(path, providers=['CPUExecutionProvider'])
    inp = session.get_inputs()[0]
    assert inp.shape == [1, 3, 768, 768] and inp.type == 'tensor(float)'
    depth = session.run(None, {
        inp.name: np.full(inp.shape, 114 / 255.0, dtype=np.float32)
    })[0]
    assert depth.shape == (1, 1, 768, 768)
    assert np.isfinite(depth).all() and (depth > 0).all()
    print(path, 'verified')
PY
```

Use the `-depth` models. The existing `yolo26n.onnx` through `yolo26x.onnx` files are object detectors. For a different inference resolution, export again with an image size divisible by 32. Static ONNX input dimensions override `vModelInputSize`; that setting supplies the size for models with dynamic spatial dimensions.

## Build and preview

Install the ONNX Runtime C/C++ dependency as described in [YOLO26 detection setup](YOLO26detectONNX.md#dependencies), then enable the same detector and vision components:

```bash
cmake -S . -B build \
  -DUSE_OPENCV=ON -DUSE_ONNXRUNTIME=ON \
  -DWITH_DETECTOR=ON -DWITH_VISION=ON -DWITH_UI=ON
cmake --build build -j
./build/OpenKAI jsonCfg/YOLO26depthEst.json
```

[The example configuration](../jsonCfg/YOLO26depthEst.json) connects a camera to the estimator and previews RGB and estimated depth in `_OCVwindow`. `_D2RGB` colorizes the depth for display; the data outputs retain metric float values. Select the camera device and replace its example intrinsics with your camera calibration before using the cloud for geometry.

## Data and calibration

| Setting | Meaning |
| --- | --- |
| `RGBframeIn` | Source RGBframe containing the usual OpenCV BGR camera image. |
| `RGBDframeOut` | Required RGBDframe receiving aligned RGB and float depth in metres. |
| `DframeOut` | Optional RGBframe carrying the same float depth map to `_D2RGB` for the window preview. |
| `PCLframeOut` | PCLframe required when `bPCL` or `bPCLrgb` is enabled. |
| `bPCL`, `bPCLrgb` | Enable cloud output; `bPCLrgb` uses source RGB colors, otherwise points are white. |
| `vFocal` | Calibrated `[fx, fy]` in pixels, both positive. |
| `vPrincipal` | Calibrated `[cx, cy]` in pixels. |
| `vSizeCalib` | `[width, height]` of the image used for calibration; intrinsics scale to the incoming frame. |
| `nPCLstep` | Pixel sampling stride for cloud generation; 1 includes every valid pixel. |
| `dScale`, `dOfs` | Optional correction `depth_metres = predicted_depth * dScale + dOfs`. Defaults are 1 and 0. |
| `vRangeD` | Accepted depth interval in metres; invalid or out-of-range depth becomes 0 and is omitted from clouds. |
| `nThread` | ONNX Runtime CPU inference workers: 0 (default) selects the physical-core pool; a positive value limits the count. |
| `thread.FPS` | Target rate for the main depth detection loop. |
| `threadPP` | Point-cloud worker thread settings. This worker wakes on completed depth estimates; `threadPP.FPS` does not cap its processing rate. |

Preprocessing uses centered letterboxing with value 114, BGR-to-RGB conversion (`bSwapRB: true`), and a `1/255` scale. The official exported depth head already applies its exponential and checkpoint calibration. Its output is metric depth; OpenKAI removes letterbox padding and resizes it back to the source image without applying another exponential or per-frame normalization. See the [Ultralytics depth head](https://github.com/ultralytics/ultralytics/blob/main/ultralytics/nn/modules/head.py) and [depth predictor](https://github.com/ultralytics/ultralytics/blob/main/ultralytics/models/yolo/depth/predict.py).

Point clouds use the pinhole equations `X = (u - cx) * Z / fx`, `Y = (v - cy) * Z / fy`, with `Z` the estimated depth in metres: X points right, Y down, and Z forward. Camera intrinsics determine the rays; they do not make monocular depth physically measured. Validate metric scale against known distances in your operating scene and adjust `dScale`/`dOfs` or calibrate the model when needed. Use an undistorted camera image with matching intrinsics when lens distortion is significant.

The published data uses the source frame timestamp so downstream components can apply their normal freshness checks. Zero means invalid depth. Consumers can use the same DataObject interfaces as hardware depth cameras, while accounting for monocular estimation error and inference latency.

Depth inference and RGBD/depth publication run in the main detector thread. When cloud output is enabled, each completed estimate wakes the `m_pTpp` worker to run `makePointCloud()` on a matching RGB/depth snapshot. The main thread can start the next inference while the worker builds and publishes the cloud. Cloud output therefore arrives after its depth output and retains the same source capture timestamp; consumers that need corresponding outputs should match timestamps.

The worker keeps at most one pending snapshot alongside the frame it is processing. If inference finishes multiple frames while the worker is busy, the newest replaces the pending frame; the in-progress frame stays intact. This bounds queued work, so cloud output can skip depth frames under load. Pausing the main detector stops new estimates, while queued or in-progress cloud work may finish. Configuration/model reload and destruction stop and join both threads before changing shared state.

Official model and export documentation: [Ultralytics monocular depth estimation](https://docs.ultralytics.com/tasks/depth/).

## Regression tests

The standalone tests use small synthetic ONNX fixtures and require no camera or Python:

```bash
cmake -S test/Detector -B /tmp/openkai-yolo26-depth-tests
cmake --build /tmp/openkai-yolo26-depth-tests -j4
ctest --test-dir /tmp/openkai-yolo26-depth-tests --output-on-failure
```

To also exercise all five real models through the C++ inference and DataObject pipeline:

```bash
/tmp/openkai-yolo26-depth-tests/yolo26_depth --model \
  /home/kai/dev/models/YOLO26/yolo26{n,s,m,l,x}-depth.onnx
```

## CPU performance

The detector enables `ORT_ENABLE_ALL`, including CPU layout optimizations, and defaults to `nThread: 0` for automatic physical-core threading. The example configuration uses this setting. Set a positive count, such as 4 or 8, when sharing the CPU with other busy modules; measure the full pipeline because the best count depends on CPU topology and workload. `thread.FPS` controls inference loop pacing. The separate point-cloud worker wakes immediately when an estimate is ready and can overlap the next inference; its `threadPP.FPS` setting does not introduce a wait or rate limit.

The following historical results were measured before the asynchronous point-cloud worker was added, when cloud generation ran synchronously in `detect()`. On the development Intel Core i9-14900KF, with C++ ONNX Runtime 1.26.0, the nano FP32 768x768 model, synthetic 640x480 BGR input, and colored point clouds at stride 2, these are medians after two warmups over five frames. Camera acquisition and GUI display are excluded; full detector time includes frame copies, preprocessing, inference, depth restoration, point-cloud generation, and output publication. The standalone benchmark uses one OpenCV worker for repeatability.

| Configuration | Inference only | Full detector | Processing FPS |
| --- | ---: | ---: | ---: |
| Original Debug, extended graph optimization, 1 worker | 455.9 ms | 507.0 ms | 1.97 |
| Updated Debug, all graph optimizations, automatic workers | 45.8 ms | 74.1 ms | 13.49 |
| Updated RelWithDebInfo, all graph optimizations, automatic workers | 48.1 ms | 61.7 ms | 16.20 |

These are local processing benchmarks, not guaranteed live-camera frame rates. The optimized graph/thread sweep changed depth values by at most about 0.00053% relative on the test tensor. Input resolution and model precision are unchanged. Scalar arithmetic also reduced Debug point-cloud generation from about 20.5 ms to 4.2 ms. `_D2RGB` now skips unchanged frames rather than repeatedly colorizing them at the preview thread rate.

The standard thread console truncates FPS to an integer, so the original 507 ms cycle appeared as 1 FPS. Use the standalone benchmark below to measure processing latency independently of loop pacing and display refresh.

The [Ultralytics depth benchmark](https://docs.ultralytics.com/tasks/depth/) reports warmed inference alone on a 32-core Xeon Skylake, excluding preprocessing and output work. Its [published ONNX profiler](https://docs.ultralytics.com/reference/utils/benchmarks/#ultralytics.utils.benchmarks.ProfileModels.profile_onnx_model) uses all graph optimizations and eight workers; the depth table does not specify its worker count. See ONNX Runtime's [graph optimization](https://onnxruntime.ai/docs/performance/model-optimizations/graph-optimizations.html) and [CPU threading](https://onnxruntime.ai/docs/performance/tune-performance/threading.html) documentation.

To benchmark the current pipeline without changing the application's existing build mode:

```bash
cmake -S test/Detector -B /tmp/openkai-depth-perf -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build /tmp/openkai-depth-perf -j4
/tmp/openkai-depth-perf/depth_benchmark \
  --model /home/kai/dev/models/YOLO26/yolo26n-depth.onnx \
  --threads 0 --graph default --warmup 2 --iterations 10
```

Use `--threads 8` to compare a bounded pool. `--stage detect` reports `detect_and_enqueue`, the main-thread time through depth publication and cloud handoff, and `detect_to_point_cloud`, the time through publication of the matching asynchronous cloud. The benchmark waits for that cloud outside the main-thread timing before starting each sample; these are per-frame latency measurements, not steady throughput with overlapping frames.

The benchmark also exposes `--graph extended` to compare the previous runtime optimization level. For application throughput beyond the Debug results, build OpenKAI with `-DCMAKE_BUILD_TYPE=RelWithDebInfo`; this retains debug symbols while optimizing the per-pixel C++ work.
