# Dear ImGui 3D Viewer

`ImGUIviewer` is a lightweight viewer module for OpenKAI 3D geometry streams. It uses the existing `_GeometryBase`, `_PCstream`, and `_PCgridBase` data path, but renders through Dear ImGui instead of Open3D's GUI.

## Build Options

The viewer is optional and is enabled with:

```bash
mkdir -p build
cd build
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=/path/to/imgui \
  -DIMGUI_BACKEND=GLFW \
  -DIMGUI_RENDERER=OPENGL \
  ../
make -j$(nproc)
```

`USE_OPEN3D` is still required because the current 3D geometry base stores point clouds and line sets with Open3D geometry types. The new viewer does not use Open3D's GUI.

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
mkdir -p build
cd build
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
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
mkdir -p build-imgui-pi
cd build-imgui-pi
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=$HOME/src/imgui \
  -DIMGUI_BACKEND=SDL2 \
  -DIMGUI_RENDERER=OPENGLES \
  ../
make -j$(nproc)
```

Optional fixed-path install:

```bash
export IMGUI_SRC=$HOME/src/imgui
export IMGUI_PREFIX=/opt/imgui

sudo mkdir -p $IMGUI_PREFIX/include $IMGUI_PREFIX/include/backends $IMGUI_PREFIX/lib
sudo cp $IMGUI_SRC/*.h $IMGUI_PREFIX/include/
sudo cp $IMGUI_SRC/backends/imgui_impl_glfw.h $IMGUI_PREFIX/include/backends/
sudo cp $IMGUI_SRC/backends/imgui_impl_sdl2.h $IMGUI_PREFIX/include/backends/
sudo cp $IMGUI_SRC/backends/imgui_impl_opengl3.h $IMGUI_PREFIX/include/backends/
sudo cp $IMGUI_SRC/backends/imgui_impl_glfw.cpp $IMGUI_PREFIX/include/backends/
sudo cp $IMGUI_SRC/backends/imgui_impl_sdl2.cpp $IMGUI_PREFIX/include/backends/
sudo cp $IMGUI_SRC/backends/imgui_impl_opengl3.cpp $IMGUI_PREFIX/include/backends/

mkdir -p /tmp/imgui-build
cd /tmp/imgui-build
c++ -std=c++17 -I$IMGUI_SRC -c \
  $IMGUI_SRC/imgui.cpp \
  $IMGUI_SRC/imgui_draw.cpp \
  $IMGUI_SRC/imgui_tables.cpp \
  $IMGUI_SRC/imgui_widgets.cpp
ar rcs libimgui.a imgui.o imgui_draw.o imgui_tables.o imgui_widgets.o
sudo cp /tmp/imgui-build/libimgui.a $IMGUI_PREFIX/lib/
```

Then configure OpenKAI with:

```bash
mkdir -p build-imgui-installed
cd build-imgui-installed
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_INCLUDE_DIR=/opt/imgui/include \
  -DIMGUI_BACKENDS_DIR=/opt/imgui/include/backends \
  -DIMGUI_LIBRARIES=/opt/imgui/lib/libimgui.a \
  -DIMGUI_BACKEND=GLFW \
  -DIMGUI_RENDERER=OPENGL \
  ../
make -j$(nproc)
```

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

Ubuntu desktop example:

```bash
sudo apt-get install libglfw3-dev libgl1-mesa-dev
mkdir -p build-imgui
cd build-imgui
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=$HOME/src/imgui \
  -DIMGUI_BACKEND=GLFW \
  -DIMGUI_RENDERER=OPENGL \
  ../
make -j$(nproc)
```

Raspberry Pi / embedded Linux example:

```bash
sudo apt-get install libsdl2-dev libgles2-mesa-dev
mkdir -p build-imgui-pi
cd build-imgui-pi
cmake \
  -DWITH_3D=ON \
  -DUSE_OPEN3D=ON \
  -DUSE_IMGUI=ON \
  -DIMGUI_DIR=$HOME/src/imgui \
  -DIMGUI_BACKEND=SDL2 \
  -DIMGUI_RENDERER=OPENGLES \
  ../
make -j$(nproc)
```

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
    "geometry": [
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

## Sample PLY Test

`jsonCfg/ImGUI.json` is configured to load:

```text
data/PointCloud/StanfordBunny/bun000.ply
```

The sample is `bun000.ply`, a 1.9 MB Stanford Bunny range scan mirror from the University of New Mexico public directory:

```bash
curl -L https://www.cs.unm.edu/~angel/TEST/CODE/Code/BUNNY/data/bun000.ply \
  -o data/PointCloud/StanfordBunny/bun000.ply
```

Run the viewer from the repository root so the relative PLY path resolves:

```bash
./build-imgui/OpenKAI jsonCfg/ImGUI.json
```

## Controls

- Left drag: orbit camera.
- Right or middle drag: pan.
- Mouse wheel: zoom.
- `Reset camera`: restore configured camera pose.
- `Auto bound`: fit the camera to the current geometry snapshot.

## Notes

This first viewer is intentionally simple: points and grid lines are projected into the ImGui draw list. That keeps the dependency surface small and makes backend compatibility easier. If very large point clouds need higher frame rates later, the same class can keep its module/API shape while replacing draw-list points with backend VBO rendering.
