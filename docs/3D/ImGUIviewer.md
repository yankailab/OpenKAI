# Dear ImGui 3D Viewer

`ImGUIviewer` is a lightweight viewer module for OpenKAI 3D geometry streams. It derives from `_GeometryViewerBase`, reads geometry through `_GeometryBase::get()` point/line ring buffers, and renders through Dear ImGui without using Open3D viewer APIs.

## Dear ImGui Install

Dear ImGui is normally consumed directly from source. The recommended setup is to clone it once and pass its source root to OpenKAI with `IMGUI_DIR`.

Clone Dear ImGui:

```bash
git clone --depth 1 https://github.com/ocornut/imgui.git
```

For Ubuntu desktop with GLFW/OpenGL:

```bash
sudo apt-get update
sudo apt-get install libglfw3-dev libgl1-mesa-dev

cd OpenKAI/build
cmake \
  -DWITH_3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=$HOME/dev/imgui \
  -DIMGUI_BACKEND=GLFW \
  -DIMGUI_RENDERER=OPENGL \
  ../
make -j$(nproc)
```

For Raspberry Pi / embedded Linux with SDL2/OpenGL ES:

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libgles2-mesa-dev

cd OpenKAI/build
cmake \
  -DWITH_3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=$HOME/dev/imgui \
  -DIMGUI_BACKEND=SDL2 \
  -DIMGUI_RENDERER=OPENGLES \
  ../
make -j$(nproc)
```


## Build Options

The viewer is optional and is enabled with:

```bash
mkdir -p build
cd build
cmake \
  -DWITH_3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=/path/to/imgui \
  -DIMGUI_BACKEND=GLFW \
  -DIMGUI_RENDERER=OPENGL \
  ../
make -j$(nproc)
```

`USE_OPEN3D` is not required for the viewer. It is only needed when you use optional Open3D-backed modules such as `_PCfile`, crop/downsample filters, or registration.

When using `IMGUI_DIR`, OpenKAI compiles ImGui and the selected backend sources directly. When using `IMGUI_INCLUDE_DIR` and `IMGUI_LIBRARIES`, OpenKAI links the core ImGui library and compiles backend sources if their `.cpp` files are available in `IMGUI_BACKENDS_DIR`.

Available backend options:

| Option | Values | Notes |
| --- | --- | --- |
| `IMGUI_BACKEND` | `GLFW`, `SDL2` | Window/input backend. `GLFW` is a good Ubuntu desktop default. `SDL2` is useful on embedded Linux images where SDL is already integrated. |
| `IMGUI_RENDERER` | `OPENGL`, `OPENGLES` | `OPENGL` targets desktop GL. `OPENGLES` targets OpenGL ES 3 style contexts, usually better for Raspberry Pi and embedded GPUs. |
| `IMGUI_DIR` | path | Dear ImGui source root containing `imgui.cpp` and `backends/`. This is the recommended setup. |
| `IMGUI_INCLUDE_DIR` | path | Alternative include directory when using a packaged/prebuilt ImGui. |
| `IMGUI_BACKENDS_DIR` | path | Alternative backend include/source directory when using a packaged/prebuilt ImGui. |
| `IMGUI_LIBRARIES` | libraries | Optional prebuilt ImGui library or libraries. If omitted and `IMGUI_DIR` is set, OpenKAI compiles the ImGui source files directly. |


## Module Config

Use the class name `ImGUIviewer` in JSON:

```json
{
  "viewer": {
    "class": "ImGUIviewer",
    "vWinSize": [1280, 720],
    "bShowPanel": true,
    "bShowGrid": true,
    "camFov": 70.0,
    "vCamLookAt": [0.0, 0.0, 0.0],
    "vCamEye": [0.0, -8.0, 4.0],
    "vCamUp": [0.0, 0.0, 1.0],
    "thread": {
      "FPS": 30
    },
    "threadUI": {
      "FPS": 60
    },
    "vGeometry": [
      {
        "_GeometryBase": "lidar_points",
        "bStatic": false,
        "nP": 200000,
        "matPointSize": 2,
        "matCol": [1.0, 1.0, 1.0, 1.0]
      },
      {
        "_GeometryBase": "occupancy_grid",
        "bStatic": true,
        "matLineWidth": 1,
        "matCol": [0.3, 0.8, 1.0, 0.75]
      }
    ]
  }
}
```

The viewer also accepts the older `geometry` object/array and `vGeometryBase` name-list keys, but `vGeometry` is the preferred key because it matches `_GeometryViewerBase`.

## Sample PLY Test

`jsonCfg/ImGUI.json` is configured to load:

```text
data/PointCloud/StanfordBunny/bun000.ply
```

This sample uses `_PCfile` for PLY file I/O, so build it with `USE_OPEN3D=ON` if you want to run that exact config. Runtime point-cloud or line modules that fill `GEOMETRY_RINGBUF` data can be viewed without Open3D. The sample is `bun000.ply`, a 1.9 MB Stanford Bunny range scan mirror from the University of New Mexico public directory:

```bash
curl -L https://www.cs.unm.edu/~angel/TEST/CODE/Code/BUNNY/data/bun000.ply \
  -o data/PointCloud/StanfordBunny/bun000.ply
```

Run the viewer from the repository root so the relative PLY path resolves:

```bash
./build/OpenKAI jsonCfg/ImGUI.json
```

## Controls

- Left drag: orbit camera.
- Right or middle drag: pan.
- Mouse wheel: zoom.
- `Reset camera`: restore configured camera pose.
- `Auto bound`: fit the camera to the current geometry snapshot.

## Notes

Point size and line width are viewer/material settings (`matPointSize`, `matLineWidth`, `pointScale`, and `lineScale`); individual `GEOMETRY_POINT` and `GEOMETRY_LINE` records only carry geometry, color, and timestamp data.

When OpenGL or OpenGL ES rendering is enabled, the viewer uploads point and line snapshots into GPU buffers and renders them from an ImGui callback. The CPU draw-list path remains available as a fallback for backends without GL support.
