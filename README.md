# EpochGUI Rounded Corners Demo

A small C++23/OpenGL demonstration of the optional rounded-rectangle geometry provided by [`Autodidac/EpochGui`](https://github.com/Autodidac/EpochGui).

This repository contains only the demo renderer and native application hosts. It does not duplicate the rounded-rectangle module, implementation, or geometry tests.

## EpochGui feature used

The demo enables:

```text
-DEPOCHGUI_ENABLE_ROUNDED_RECT=ON
```

and imports:

```cpp
import epoch.gui.rounded_rect;
```

The optional EpochGui module supplies renderer-neutral meshes for:

- Filled rounded rectangles
- Independent radius per corner
- Optional borders with configurable thickness
- Pill-shaped controls
- Proportional radius normalization when corners overlap
- Configurable tessellation from 1 to 64 segments per corner

The full API and build instructions live in the EpochGui repository README.

## Demo application

The demo uploads the EpochGui-generated vertices and indices through one shared OpenGL 3.2 core renderer with native hosts:

- Windows: Win32 and WGL
- Linux: X11 and GLX
- macOS: Cocoa and the system OpenGL framework

The application renders complete meshes normally. It contains no startup delay, progressive border reveal, or timing behavior.

No third-party windowing or OpenGL-loader library is used.

## Repository layout

```text
CMakePresets.json                 Windows and Linux configure/build presets
build_msvc.bat/.ps1               Generate, build, test, and locate the Windows executable
generate_msvc.ps1                 Select Visual Studio 2026 or 2022 automatically
open_msvc.bat/.ps1                Generate and open the Visual Studio solution
build_linux.sh                    Configure, build, and test a Linux preset
include/                          Native host bridge
modules/epoch.gui.demo.opengl.ixx Demo renderer module
src/opengl_renderer.cpp           OpenGL submission of EpochGui meshes
src/platform/                     Native Windows, Linux, and macOS hosts
```

## EpochGui checkout

Place the repositories beside one another:

```text
Projects/
|-- EpochGui/
`-- EpochGuiRoundedCorners/
```

The demo finds `../EpochGui` automatically.

For another layout, pass:

```text
-DEPOCHGUI_ROOT=/path/to/EpochGui
```

## Build

### Windows

Generate and open the best installed Visual Studio version:

```text
open_msvc.bat
```

Build the application and run the EpochGui tests:

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
cmake -S . -B build/macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DEPOCHGUI_ROOT=/path/to/EpochGui \
  -DBUILD_TESTING=ON
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
open build/macos/EpochGuiRoundedCorners.app
```

## Windows release

The release ZIP contains exactly:

```text
EpochGuiRoundedCorners.exe
README.md
BUILD.txt
```

The workflow builds against `Autodidac/EpochGui`, runs its tests, verifies that the archive contains one executable, and publishes a SHA-256 checksum beside the ZIP.
