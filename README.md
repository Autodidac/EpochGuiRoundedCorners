# EpochGUI Rounded Corners Demo

A small C++23/OpenGL demonstration of the optional rounded-rectangle geometry now provided directly by EpochGUI.

This repository no longer owns a rounded-rectangle module or geometry implementation. It enables `EPOCHGUI_ENABLE_ROUNDED_RECT`, links the EpochGUI target from an EpochEngine checkout, imports `epoch.gui.rounded_rect`, and renders the resulting meshes.

## What EpochGUI provides

The optional `epoch.gui.rounded_rect` module supports:

- Uniform or independent radius for each corner
- Filled rounded rectangles
- Optional borders with configurable thickness
- Pill-shaped controls
- Proportional radius reduction when adjacent corners overlap
- Configurable tessellation from 1 to 64 segments per corner
- Sanitization of negative and non-finite input
- Deterministic vertices and triangle indices

`RoundedRectMesh` contains vertex positions, fill indices, optional border indices, normalized bounds and radii, and outer/inner contour ranges. It is renderer-neutral.

Enable it when configuring EpochGUI:

```text
-DEPOCHGUI_ENABLE_ROUNDED_RECT=ON
```

Then import it:

```cpp
import epoch.gui.rounded_rect;

namespace rounded = epochengine::gui_lib::rounded_rect;

const rounded::RoundedRectMesh mesh = rounded::make_rounded_rect_mesh({
    .bounds = { { 40.0f, 40.0f }, { 240.0f, 96.0f } },
    .radii = { 18.0f, 18.0f, 18.0f, 18.0f },
    .border_width = 3.0f,
    .segments_per_corner = 12
});
```

## Demo application

The demo submits EpochGUI-generated meshes through one shared OpenGL 3.2 core renderer and native platform hosts:

- Windows: Win32 and WGL
- Linux: X11 and GLX
- macOS: Cocoa and the system OpenGL framework

The application renders the complete meshes normally. There is no startup delay or progressive border animation.

No third-party windowing or OpenGL-loader library is used.

## Windows release

The Windows release ZIP contains:

```text
EpochGuiRoundedCorners.exe
README.md
BUILD.txt
```

The geometry contract test runs during the build and is not included in the archive. A SHA-256 checksum is published beside the ZIP.

## Dependencies

- EpochEngine checkout containing EpochGUI
- CMake 3.28 or newer
- C++23 compiler with module support
- Visual Studio 2022/2026 or Ninja
- Platform OpenGL/window-system development components

The project does not use GLFW, SDL, SFML, GLAD, GLEW, or a package-manager runtime dependency.

## Repository layout

```text
CMakePresets.json                 Windows and Linux configure/build presets
build_msvc.bat/.ps1               Generate, build, test, and locate the Windows executable
generate_msvc.ps1                 Select Visual Studio 2026 or 2022 automatically
open_msvc.bat/.ps1                Generate and open the Visual Studio solution
build_linux.sh                    Configure, build, and test a Linux preset
include/                          Native host bridge
modules/epoch.gui.demo.opengl.ixx Demo renderer module only
src/opengl_renderer.cpp           OpenGL submission of EpochGUI meshes
src/platform/                     Native platform hosts
tests/geometry_contract.cpp       EpochGUI rounded-geometry contract test
```

## EpochEngine checkout

The repositories can be sibling directories:

```text
Projects/
|-- EpochEngine/
|   `-- Engine/
`-- EpochGuiRoundedCorners/
```

For another layout, pass:

```text
-DEPOCH_ENGINE_ROOT=/path/to/EpochEngine/Engine
```

## Build

### Windows

Generate and open the best installed Visual Studio version:

```text
open_msvc.bat
```

Build and run the contract tests:

```text
build_msvc.bat Release
```

The script selects Visual Studio 2026 when available, otherwise Visual Studio 2022.

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
cmake -S . -B build/macos -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
open build/macos/EpochGuiRoundedCorners.app
```
