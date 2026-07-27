# EpochGui Demo

A cross-platform C++23 live catalog for [`Autodidac/EpochGui`](https://github.com/Autodidac/EpochGui).

The application demonstrates every subsystem currently exported by the core `epoch.gui` module, plus the optional rounded-rectangle and fallback-input modules. It is an interactive workspace, not a static screenshot.

## Pages

The custom EpochGui toolbar switches between five live pages.

### Overview

Demonstrates:

- `Vec2`, `Rect`, and `contains`
- Segmented-control layout and hit testing
- Selectable-list ranges, rows, hover, and selection
- Progress bars in all four directions
- Loading-screen layout
- A draggable splitter with before, handle, and after regions

### Windows

Demonstrates:

- Real draggable and resizable `FloatingWindowState` windows
- Working floating-window close buttons
- Reopening closed windows from the right-click context menu
- Focus ordering for overlapping windows
- Core rectangular and optional rounded rendering paths
- `DockLayoutState` with left, center, and right panes
- Pane popout and redock behavior
- `DockableWindowState` and dock, float, and detach actions
- `PanelHostState` and docked/floating hosting modes
- Cursor-anchored `PopupState` context menus

### Text

Demonstrates:

- Editable single-line text control
- Editable multiline text control
- Read-only text control used as static text
- Native text-event insertion
- Caret placement and movement
- Backspace, Delete, arrows, Home, End, Enter, and Tab behavior
- Selection, clipboard, scrolling, read-only, and byte-limit APIs exposed by EpochGui

### Image

Demonstrates actual image loading and display:

- Loads `assets/epochgui_demo.ppm` at runtime
- Displays the loaded image inside an EpochGui `Rect`
- Toggles between aspect-fit and stretch modes
- Uses EpochGui layout and hit testing while keeping file decoding and rendering in the demo layer

EpochGui itself remains backend-neutral and does not become an asset loader.

### Input

Demonstrates:

- Live pointer-button state
- Pointer movement and wheel input
- Keyboard and modifier state
- Right-click context requests
- `InputTracker`
- Borderless/custom-chrome layout and hit testing
- Caption, client, resize-edge, resize-corner, minimize, maximize, and close regions

## Custom title bar and native frame

The custom EpochGui title bar is **always visible**.

The native Windows outer frame is enabled by default. The toolbar toggle switches only the outer host presentation:

- **Native frame on:** ordinary Windows frame plus the custom EpochGui application bar
- **Borderless mode:** custom EpochGui title bar and its native move/resize/command mapping without the standard outer frame

The custom minimize, maximize, and close buttons remain active in both modes. On Windows, the custom caption and resize regions are mapped through `WM_NCHITTEST`, retaining native moving, snapping, and resizing behavior.

Both the toolbar switch and the right-click context-menu switch use the same deferred host path. The Win32 window is created once with `WS_OVERLAPPEDWINDOW` and keeps that style for its entire lifetime. Toggling changes only custom non-client calculation and painting behavior, then requests `SWP_FRAMECHANGED` after the current input/render callback has returned. The code does not call `SetWindowLongPtr(GWL_STYLE)` at runtime, so the HWND, device context, pixel format, and OpenGL context remain stable.

## Optional fallback input

Input is disabled by default in EpochGui:

```text
EPOCHGUI_ENABLE_INPUT=OFF
```

Applications that already use engine, SDL, GLFW, platform, or another input system should continue using it.

This standalone demo explicitly enables the fallback:

```text
-DEPOCHGUI_ENABLE_INPUT=ON
```

and imports:

```cpp
import epoch.gui.input;
```

The fallback only normalizes per-frame input and translates it into existing EpochGui widget inputs. Native Win32, X11, and Cocoa hosts still deliver events and execute platform window commands.

## Optional rounded geometry

Rounded geometry also remains disabled unless requested:

```text
-DEPOCHGUI_ENABLE_ROUNDED_RECT=ON
```

The Windows page renders the same floating-window layouts through both paths:

- Core rectangular surfaces from `epoch.gui`
- Optional rounded meshes from `epoch.gui.rounded_rect`

## EpochGui API coverage

The demo exercises the public APIs for:

- Floating windows
- Splitters
- Progress bars
- Loading-screen layouts
- Selectable lists
- Segmented controls
- Popups
- Dock layouts
- Dockable windows
- Text controls
- Panel hosting
- Rounded-rectangle meshes
- Fallback input and custom window chrome

## Architecture

EpochGui owns reusable state, layout, geometry, hit testing, text editing, docking metadata, panel-host transitions, optional rounded meshes, and optional normalized fallback input.

This repository owns only:

- Native Win32/WGL, X11/GLX, and Cocoa event translation
- OpenGL 3.2 core presentation
- Demo-side text-event forwarding
- PPM image loading and display
- Native frame presentation and host window commands
- A compact bitmap font
- Build scripts and release packaging

No EpochGui implementation is duplicated in the demo.

## EpochGui checkout

Keep the repositories beside one another:

```text
Projects/
|-- EpochGui/
`-- EpochGui_Demo/
```

The demo detects `../EpochGui` automatically. For another layout:

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

Outputs include:

```text
build/vs2026/Release/EpochGuiDemo.exe
build/vs2026/Release/assets/epochgui_demo.ppm
```

or the corresponding `build/vs2022/Release` paths.

### Linux

```bash
chmod +x build_linux.sh
./build_linux.sh linux-gcc-release
```

or:

```bash
./build_linux.sh linux-clang-release
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
CMakeLists.txt                         Demo targets and optional EpochGui features
CMakePresets.json                      Windows and Linux presets
assets/epochgui_demo.ppm               Runtime-loaded demonstration image
build_msvc.bat/.ps1                    Generate, build, test, and locate the executable
generate_msvc.ps1                     Select Visual Studio 2026 or 2022
open_msvc.bat/.ps1                    Generate and open the solution
build_linux.sh                        Configure, build, and test Linux presets
include/epochgui_demo/                Native renderer/input/text bridge
modules/epoch.gui.demo.opengl.ixx     Complete renderer module interface
src/complete_demo_renderer.cpp        Full interactive EpochGui catalog
src/platform/complete_windows_main.cpp Framed Windows host and runtime frame toggle
src/platform/                         Linux and macOS hosts
```

## Windows release

The release ZIP contains:

```text
EpochGuiDemo.exe
README.md
BUILD.txt
assets/epochgui_demo.ppm
```

The workflow checks out the current standalone EpochGui repository, enables rounded rectangles and fallback input, builds both projects, runs all enabled EpochGui tests, verifies that exactly one executable and the required image asset are present, and publishes a SHA-256 checksum beside the ZIP.
