# EpochGUI Rounded Corners

A standalone C++23 reference implementation for adding rounded rectangles to EpochGUI.

The project generates reusable triangle meshes for filled and bordered rounded rectangles, then renders those meshes with a small OpenGL demo. It is kept outside EpochEngine so the geometry, API, and rendering behavior can be tested before being moved into the production GUI library.

## Features

- Uniform or independent radius for each corner
- Filled rounded rectangles
- Rounded borders with configurable thickness
- Pill-shaped controls
- Automatic proportional radius reduction when adjacent corners overlap
- Configurable curve tessellation from 1 to 64 segments per corner
- Sanitization of negative and non-finite input
- Deterministic vertices and triangle indices
- Headless mesh validation and SVG reference output

## Architecture

### Rounded-rectangle geometry

`epoch.gui.rounded_rect` is the reusable portion of the project. It imports EpochGUI and operates on its existing `Vec2` and `Rect` types.

The generated `RoundedRectMesh` contains:

- Vertex positions
- Fill triangle indices
- Border triangle indices
- Normalized bounds, radii, and border width
- Outer and inner contour ranges

The mesh is renderer-independent and can be submitted through any EpochGUI backend.

### OpenGL demo renderer

The interactive application uses a shared OpenGL 3.2 core renderer with VAOs, VBOs, index buffers, and small GLSL shaders.

Each operating system provides only the native window and OpenGL context:

- Windows: Win32 and WGL
- Linux: X11 and GLX
- macOS: Cocoa and the system OpenGL framework

OpenGL functions are loaded directly through the platform context API. No third-party windowing or OpenGL loader library is used.

### Headless validator

`epoch_gui_rounded_corners_demo` validates the generated meshes and writes an SVG showing the same rounded-corner cases. This provides a renderer-independent test for geometry changes.

## Dependencies

Project dependency:

- EpochGUI from an EpochEngine checkout

Build requirements:

- CMake 3.28 or newer
- A C++23 compiler with module support
- Visual Studio 2022 or Ninja

Platform development components:

- Windows: Windows SDK and `opengl32`
- Linux: X11 and OpenGL development headers
- macOS: Cocoa and OpenGL frameworks supplied by macOS

The project does not require GLFW, SDL, SFML, GLAD, GLEW, or another package manager dependency.

## Repository layout

```text
include/                         Native host bridge
modules/                         C++23 module interfaces
src/rounded_rect.cpp             Rounded geometry implementation
src/opengl_renderer.cpp          Shared OpenGL renderer
src/headless_main.cpp            Mesh validator and SVG generator
src/platform/                    Native Windows, Linux, and macOS hosts
```

## EpochEngine checkout

The build imports EpochGUI from:

```text
<EPOCH_ENGINE_ROOT>/dep/EpochGui/modules/epoch.gui.ixx
```

When both repositories are sibling directories, the path is detected automatically:

```text
Projects/
|-- EpochEngine/
|   `-- Engine/
`-- EpochGuiRoundedCorners/
```

For another layout, pass the engine directory explicitly:

```text
-DEPOCH_ENGINE_ROOT=/path/to/EpochEngine/Engine
```

## Build

### Windows

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\EpochGuiRoundedCorners.exe
```

### Linux

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/EpochGuiRoundedCorners
```

### macOS

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
open build/EpochGuiRoundedCorners.app
```

## Integrating into EpochGUI

The intended production integration is:

1. Add `epoch.gui.rounded_rect` and `rounded_rect.cpp` to the EpochGUI library.
2. Expose a rounded-rectangle draw command through the public GUI API.
3. Submit the generated vertices and indices through each existing rendering backend.
4. Keep the current rectangular path as the zero-radius fast path.
5. Cache meshes by dimensions, corner radii, border width, and tessellation level.

Only the geometry module is intended to become part of EpochGUI. The native hosts, OpenGL demo renderer, and SVG generator are test and reference applications.
