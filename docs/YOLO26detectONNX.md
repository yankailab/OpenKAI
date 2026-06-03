# YOLO26detectONNX Setup

`_YOLO26detectONNX` runs Ultralytics YOLO26 detection ONNX models with ONNX Runtime for inference and OpenCV for image preprocessing and NMS fallback. It supports the default YOLO26 end-to-end output `(N, 300, 6)` and the traditional one-to-many export output `(N, nc + 4, predictions)`.

## Dependencies

Install the normal OpenKAI OpenCV dependencies first, then install ONNX Runtime C/C++.

### Ubuntu desktop x86-64

```bash
sudo apt update
sudo apt install -y build-essential cmake wget tar

ORT_VERSION=1.26.0
ORT_ARCH=x64
wget https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}.tgz
tar xf onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}.tgz
sudo rm -rf /usr/local/include/onnxruntime
sudo mkdir -p /usr/local/include/onnxruntime
sudo cp -r onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}/include/* /usr/local/include/onnxruntime/
sudo cp onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}/lib/libonnxruntime.so* /usr/local/lib/
sudo ldconfig
```

### Raspberry Pi 64-bit OS

Use the `aarch64` ONNX Runtime package on Raspberry Pi OS 64-bit or Ubuntu Server 64-bit.

```bash
sudo apt update
sudo apt install -y build-essential cmake wget tar

ORT_VERSION=1.26.0
ORT_ARCH=aarch64
wget https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}.tgz
tar xf onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}.tgz
sudo rm -rf /usr/local/include/onnxruntime
sudo mkdir -p /usr/local/include/onnxruntime
sudo cp -r onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}/include/* /usr/local/include/onnxruntime/
sudo cp onnxruntime-linux-${ORT_ARCH}-${ORT_VERSION}/lib/libonnxruntime.so* /usr/local/lib/
sudo ldconfig
```

If your Pi uses a 32-bit OS, install a 64-bit OS first or build ONNX Runtime from source for that target.

## Build OpenKAI

Enable OpenCV, detectors, vision, universe, and ONNX Runtime:

```bash
mkdir -p build
cd build
cmake .. \
  -DUSE_OPENCV=ON \
  -DUSE_ONNXRUNTIME=ON \
  -DWITH_DETECTOR=ON \
  -DWITH_VISION=ON \
  -DWITH_UNIVERSE=ON \
  -DONNXRuntime_include=/usr/local/include/onnxruntime \
  -DONNXRuntime_lib=/usr/local/lib
make -j$(nproc)
```

## Export A YOLO26 ONNX Model

Download one of the COCO detection `.pt` weights:

```bash
mkdir -p ~/dev/models
cd ~/dev/models

wget https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26n.pt
wget https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26s.pt
wget https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26m.pt
wget https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26l.pt
wget https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo26x.pt
```

For Raspberry Pi, start with `yolo26n.pt` or `yolo26s.pt`.

```bash
sudo apt install -y python3-full python3-venv

python3 -m venv ~/venv/ultralytics
source ~/venv/ultralytics/bin/activate
python3 -m pip install -U pip
python3 -m pip install -U ultralytics

python3 - <<'PY'
from ultralytics import YOLO
model = YOLO("/home/kai/dev/models/yolo26n.pt")
model.export(format="onnx", imgsz=640)
PY
```

Ubuntu 24.04 and newer protect the system Python environment. If `pip install` shows `externally-managed-environment`, use the virtual environment commands above instead of `--break-system-packages`.

The default YOLO26 export uses end-to-end detection, so `_YOLO26detectONNX` filters detections by `confidence` and does not run external NMS. If you export with `end2end=False`, `_YOLO26detectONNX` will parse the traditional output and apply OpenCV NMS using `score` and `nms`.

## Config Example

```json
"YOLO26detectONNX":{
    "class":"_YOLO26detectONNX",
    "bON": true,
"thread":{
        "FPS":30,
    },
    "bLog": true,
    "_VisionBase":"cam",
    "_Universe":"univ",
    "fModel":"/home/kai/dev/models/yolo26n.onnx",
    "confidence":0.25,
    "score":0.45,
    "nms":0.5,
    "bLetterBoxForSquare":true,
    "vModelInputSize":[640,640],
    "bSwapRB":true,
    "scale":0.003921568627,
    "nThread":1,
}
```

## Notes

- `confidence` is used for the default YOLO26 end-to-end output.
- `score` and `nms` are used for one-to-many exports with external NMS.
- `vModelInputSize` must match the ONNX export image size unless the model was exported with dynamic input shapes.
- On Raspberry Pi, start with `yolo26n.onnx` and keep `nThread` low; increasing it can help throughput on some boards but can also starve the camera pipeline.

References: [Ultralytics YOLO26](https://docs.ultralytics.com/models/yolo26/), [Ultralytics end-to-end detection guide](https://docs.ultralytics.com/guides/end2end-detection/), [ONNX Runtime install](https://onnxruntime.ai/docs/install/).
