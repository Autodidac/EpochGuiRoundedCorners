# EpochGUI Rounded Corners

A standalone C++23 reference implementation for adding rounded rectangles to EpochGUI.

The project generates renderer-independent triangle meshes for filled and bordered rounded rectangles, then displays them through a small native OpenGL application. It is kept separate from EpochEngine so the API and rendering behavior can be tested before production integration.

## Features

- Uniform or independent radius for each corner
- Filled rounded rectangles
- Rounded borders with configurable thickness
- Pill-shaped controls
- Proportional radius reduction when adjacent corners overlap
- Configurable tessellation from 1 to 64 segments per corner
- Sanitization of negative and non-finite input
- Deterministic vertices and triangle indices
- Progressive startup reveal that assembles the real border triangles segment by segment

## Startup border reveal

When the native application opens, the fills appear immediately and the four rounded borders assemble in contour order. The renderer waits briefly, then reveals one border segment every 25 milliseconds across the examples.

The animation is time-based and non-blocking. Windows, Linux, and macOS keep processing native events while it runs, then return to event-driven rendering after the reveal completes.

## Architecture

### Rounded geometry

`epoch.gui.rounded_rect` imports EpochGUI and operates on its existing `Vec2` and `Rect` types.

`RoundedRectMesh` contains:

- Vertex positions
- Fill triangle indices
- Border triangle indices
- Normalized bounds, radii, and border width
- Outer and inner contour ranges

The generated mesh can be submitted through any EpochGUI rendering backend.

### OpenGL application

The demonstration application uses one shared OpenGL 3.2 core renderer with native platform hosts:

- Windows: Win32 and WGL
- Linux: X11 and GLX
- macOS: Cocoa and the system OpenGL framework

No third-party windowing or OpenGL loader library is used.

## Windows release

The Windows release is a ZIP containing:

```text
EpochGuiRoundedCorners.exe
README.md
BUILD.txt
```

The geometry contract test is used only during the build and is not included in the release. A SHA-256 checksum is published beside the ZIP.

## Dependencies

Project dependency:

- EpochGUI from an EpochEngine checkout

Build requirements:

- CMake 3.28 or newer
- A C++23 compiler with module support
- Visual Studio 2022/2026 or Ninja

Platform components:

- Windows: Windows SDK and `opengl32`
- Linux: X11 and OpenGL development headers
- macOS: Cocoa and OpenGL frameworks supplied by macOS

The project does not use GLFW, SDL, SFML, GLAD, GLEW, or another package-manager dependency.

## Repository layout

```text
CMakePresets.json                 Windows and Linux configure/build presets
build_msvc.bat/.ps1               Generate, build, test, and locate the Windows executable
generate_msvc.ps1                 Select Visual Studio 2026 or 2022 automatically
open_msvc.bat/.ps1                Generate and open the Visual Studio solution
build_linux.sh                    Configure, build, and test a Linux preset
include/                          Native host bridge
modules/                          C++23 module interfaces
src/rounded_rect.cpp              Rounded geometry implementation
src/opengl_renderer.cpp           Shared OpenGL renderer and startup reveal
src/platform/                     Native platform hosts
tests/geometry_contract.cpp       Internal geometry contract test
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

Generate and open the best installed Visual Studio version:

```text
open_msvc.bat
```

Build and run the geometry contract test:

```text
build_msvc.bat Release
```

The script automatically selects Visual Studio 2026 when available, otherwise Visual Studio 2022. The executable is written to:

```text
build/vs2026/Release/EpochGuiRoundedCorners.exe
```

or:

```text
build/vs2022/Release/EpochGuiRoundedCorners.exe
```

### Linux

GCC 14 preset:

```bash
chmod +x build_linux.sh
./build_linux.sh linux-gcc-release
```

Clang 18 preset:

```bash
./build_linux.sh linux-clang-release
```

Clang requires a matching `clang-scan-deps` installation for C++23 module scanning.

Outputs:

```text
build/linux-gcc/EpochGuiRoundedCorners
build/linux-clang/EpochGuiRoundedCorners
```

### macOS

```bash
cmake -S . -B build/macos -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
open build/macos/EpochGuiRoundedCorners.app
```

## Integrating into EpochGUI

1. Add `epoch.gui.rounded_rect` and `rounded_rect.cpp` to the EpochGUI library.
2. Expose a rounded-rectangle draw command through the public GUI API.
3. Submit the generated vertices and indices through each rendering backend.
4. Preserve the existing rectangular path as the zero-radius fast path.
5. Cache meshes by dimensions, corner radii, border width, and tessellation level.

Only the geometry module is intended for production integration. The native hosts and OpenGL renderer are reference code.
