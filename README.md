# EpochGui Demo

A cross-platform C++23 feature gallery for [`Autodidac/EpochGui`](https://github.com/Autodidac/EpochGui).

The application renders the same EpochGui layouts in two side-by-side columns:

- **Core rectangular path** — uses the standard `epoch.gui` layout and state APIs with ordinary rectangular surfaces.
- **Optional rounded path** — uses the exact same layouts while enabling `epoch.gui.rounded_rect` for rounded fills and borders.

This makes the optional geometry visible without confusing it with the core layout system.

## Features demonstrated

The gallery uses real EpochGui calculations for:

- Segmented-control bounds and item placement
- Progress-bar track, padding, range, and fill geometry
- Splitter layout with before, handle, and after regions
- Selectable-list rows, visible ranges, hover state, and selection state
- Popup placement and viewport clamping
- Floating-window title bar, content, close button, and resize handle
- Loading-screen panel and progress layout
- Core `Vec2`, `Rect`, and containment primitives

The right column additionally demonstrates:

- Uniform rounded corners
- Rounded control borders
- Rounded panels and popup surfaces
- Pill-style progress fills
- The optional renderer-neutral `RoundedRectMesh`

## Architecture

EpochGui owns the reusable layout, state, hit-testing, and optional rounded-mesh generation.

This repository owns only:

- A small OpenGL 3.2 core renderer
- Native Win32/WGL, X11/GLX, and Cocoa hosts
- A compact built-in bitmap font used to label the gallery
- Build scripts and release packaging

The demo does not duplicate EpochGui geometry or layout implementation.

## EpochGui checkout

Keep the repositories beside one another:

```text
Projects/
|-- EpochGui/
`-- EpochGui_Demo/
```

The demo detects `../EpochGui` automatically.

For another layout, configure with:

```text
-DEPOCHGUI_ROOT=/path/to/EpochGui
```

The demo enables the optional comparison feature with:

```text
-DEPOCHGUI_ENABLE_ROUNDED_RECT=ON
```

## Build

Requirements:

- CMake 3.28 or newer
- A C++23 compiler with module support
- Visual Studio 2022/2026 or Ninja
- Native OpenGL and window-system development components

### Windows

Generate and open the best installed Visual Studio version:

```text
open_msvc.bat
```

Build the application and run the EpochGui tests:

```text
build_msvc.bat Release
```

Output:

```text
build/vs2026/Release/EpochGuiDemo.exe
```

or:

```text
build/vs2022/Release/EpochGuiDemo.exe
```

### Linux

GCC preset:

```bash
chmod +x build_linux.sh
./build_linux.sh linux-gcc-release
```

Clang preset:

```bash
./build_linux.sh linux-clang-release
```

Outputs:

```text
build/linux-gcc/EpochGuiDemo
build/linux-clang/EpochGuiDemo
```

Clang requires a matching `clang-scan-deps` installation.

### macOS

```bash
cmake -S . -B build/macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DEPOCHGUI_ROOT=/path/to/EpochGui \
  -DBUILD_TESTING=ON
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
open build/macos/EpochGuiDemo.app
```

## Repository layout

```text
CMakeLists.txt                     Demo targets and EpochGui integration
CMakePresets.json                  Windows and Linux presets
build_msvc.bat/.ps1                Generate, build, test, and locate the executable
generate_msvc.ps1                 Select Visual Studio 2026 or 2022
open_msvc.bat/.ps1                Generate and open the solution
build_linux.sh                    Configure, build, and test Linux presets
include/epochgui_demo/            Native renderer bridge
modules/epoch.gui.demo.opengl.ixx Demo renderer module
src/opengl_renderer.cpp           Side-by-side EpochGui feature gallery
src/platform/                     Native Windows, Linux, and macOS hosts
```

## Rendering boundary

The demo renderer converts EpochGui `Rect` layouts and optional `RoundedRectMesh` data into OpenGL triangles. EpochGui itself remains independent of OpenGL, platform windows, shaders, fonts, release packaging, and application-specific presentation.

## Windows release

The release ZIP contains exactly:

```text
EpochGuiDemo.exe
README.md
BUILD.txt
```

The workflow checks out the current standalone EpochGui repository, enables rounded-rectangle support, builds both projects, runs the EpochGui tests, verifies that the archive contains one executable, and publishes a SHA-256 checksum beside the ZIP.
