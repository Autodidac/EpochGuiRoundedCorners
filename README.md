# EpochGui Demo

A cross-platform C++23 live demonstration of [`Autodidac/EpochGui`](https://github.com/Autodidac/EpochGui).

The application is an interactive GUI workspace rather than a static screenshot. It uses real EpochGui state, layout, hit testing, popup placement, optional rounded geometry, and the optional fallback input adapter.

## Live interactions

The demo supports:

- Dragging independent floating windows by their EpochGui title bars
- Resizing floating windows from their lower-right handles
- Closing and reopening windows
- Focus ordering when windows overlap
- Hover and selection state in a selectable sidebar
- A live segmented control that switches between rectangular, rounded, and combined views
- Right-click context menus anchored at the cursor
- Escape and Tab keyboard behavior
- Pointer button, wheel, key, and modifier event translation
- A replacement title bar with minimize, maximize, close, caption, and resize hit regions

Right-click the workspace to open the context menu. It can reopen either floating window or reset the full layout.

## Core versus optional features

The two floating windows use the same core EpochGui layout APIs:

- **Core window:** ordinary rectangular rendering using `epoch.gui`
- **Rounded window:** the same layouts rendered with `epoch.gui.rounded_rect`

This keeps layout/state behavior separate from optional visual geometry.

## Optional fallback input

Input remains **disabled by default** in the EpochGui library:

```text
EPOCHGUI_ENABLE_INPUT=OFF
```

Applications that already have engine, platform, SDL, GLFW, or other input handling should continue using it.

This standalone demo explicitly enables the fallback:

```text
-DEPOCHGUI_ENABLE_INPUT=ON
```

and imports:

```cpp
import epoch.gui.input;
```

The fallback module only normalizes per-frame input and converts it into existing EpochGui widget inputs. It contains no platform calls. Win32, X11, and Cocoa remain responsible for delivering native events and executing native window commands.

## Borderless replacement title bar

The title bar drawn by the demo is backed by EpochGui's optional input module. EpochGui calculates:

- Caption region
- Client region
- Resize edges and corners
- Minimize button
- Maximize button
- Close button

On Windows, those regions are mapped directly through `WM_NCHITTEST`, giving the borderless application normal native moving, resizing, snapping, minimizing, maximizing, and closing behavior.

Linux uses the same regions with `_NET_WM_MOVERESIZE` and `_NET_WM_STATE`. macOS feeds the same UI state through a borderless Cocoa window and native window dragging/actions.

## EpochGui features exercised

The live workspace uses real EpochGui APIs for:

- `FloatingWindowState`, `FloatingWindowInput`, and `update_floating_window`
- `PopupState`, `PopupInput`, and `update_popup`
- Selectable-list row calculation and hit testing
- Segmented-control item placement and hit testing
- Progress-bar geometry
- Splitter layouts
- `Vec2`, `Rect`, and containment tests
- `RoundedRectMesh`
- `InputTracker`
- Right-click context requests
- Borderless window chrome layout and hit testing

## Architecture

EpochGui owns reusable layout, state, hit testing, rounded-mesh generation, and optional normalized fallback input.

This repository owns only:

- Native Win32/WGL, X11/GLX, and Cocoa event translation
- The OpenGL 3.2 core presentation layer
- Native execution of minimize, maximize, close, move, and resize commands
- A compact bitmap font for labels
- Build scripts and release packaging

No EpochGui layout, rounded geometry, or input implementation is duplicated here.

## EpochGui checkout

Keep the repositories beside one another:

```text
Projects/
|-- EpochGui/
`-- EpochGui_Demo/
```

The demo detects `../EpochGui` automatically.

For another layout:

```text
-DEPOCHGUI_ROOT=/path/to/EpochGui
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

Build the application and run all enabled EpochGui tests:

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

```bash
chmod +x build_linux.sh
./build_linux.sh linux-gcc-release
```

or:

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
CMakeLists.txt                     Demo targets and optional EpochGui features
CMakePresets.json                  Windows and Linux presets
build_msvc.bat/.ps1                Generate, build, test, and locate the executable
generate_msvc.ps1                 Select Visual Studio 2026 or 2022
open_msvc.bat/.ps1                Generate and open the solution
build_linux.sh                    Configure, build, and test Linux presets
include/epochgui_demo/            Native renderer/input bridge
modules/epoch.gui.demo.opengl.ixx Live renderer module interface
src/live_renderer.cpp             Interactive EpochGui workspace
src/platform/                     Native Windows, Linux, and macOS hosts
```

## Windows release

The release ZIP contains exactly:

```text
EpochGuiDemo.exe
README.md
BUILD.txt
```

The workflow checks out the current standalone EpochGui repository, enables rounded rectangles and fallback input, builds both projects, runs the EpochGui tests, verifies that the archive contains one executable, and publishes a SHA-256 checksum beside the ZIP.
