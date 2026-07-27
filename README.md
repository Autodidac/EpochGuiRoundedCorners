# EpochGUI Rounded Corners

Standalone C++23 prototype for backend-neutral rounded rectangle geometry using EpochGUI's existing `Vec2` and `Rect` types.

The repository is intentionally separate from EpochEngine. It does not modify the editor, renderer frame order, GUI batching, or production EpochGUI source.

## Rendering

The interactive demo uses one shared OpenGL 3.2 core renderer:

- Windows: Win32 + WGL
- Linux: X11 + GLX
- macOS: Cocoa + the system OpenGL framework

There is no GLFW, SDL, SFML, GLAD, GLEW, Direct2D, Vulkan SDK, package manager, or vendored third-party code. OpenGL functions are loaded through the native platform context API.

The only project-level dependency is EpochGUI. Native windowing and OpenGL are supplied by the operating system.

## Windows release

The Windows x64 release contains:

- `EpochGuiRoundedCorners.exe` — native OpenGL window displaying the rounded-corner cases
- `epoch_gui_rounded_corners_demo.exe` — headless geometry validator and SVG generator
- `epochgui_rounded_corners_demo.svg` — generated reference output
- `README.md`

Both executables consume the same `RoundedRectMesh` implementation.

## What it demonstrates

- Uniform and independent per-corner radii
- Pill-shaped controls
- Filled and bordered rounded rectangles
- Proportional radius normalization when adjacent radii overlap
- Bounded tessellation from 1 to 64 segments per corner
- Sanitized non-finite and negative input
- Deterministic vertex and triangle-index generation
- VAO/VBO/EBO rendering through OpenGL 3.2 core shaders
- A headless SVG output for renderer-independent validation

## Requirements for source builds

- CMake 3.28 or newer
- A C++23 compiler with module support
- Ninja or Visual Studio generator
- A local EpochEngine checkout

Linux builds require the normal X11 and OpenGL development headers. macOS uses its built-in Cocoa and OpenGL frameworks. Windows uses the Windows SDK and `opengl32`.

The project imports the real EpochGUI module from:

```text
<EPOCH_ENGINE_ROOT>/dep/EpochGui/modules/epoch.gui.ixx
```

`EPOCH_ENGINE_ROOT` is the path to the `Engine` directory inside EpochEngine.

## Build

When both repositories are sibling directories, EpochEngine is detected automatically:

```text
Projects/
|-- EpochEngine/
|   `-- Engine/
`-- EpochGuiRoundedCorners/
```

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

For another checkout layout, add:

```text
-DEPOCH_ENGINE_ROOT=/path/to/EpochEngine/Engine
```

## Production integration

After the geometry and visual result are accepted:

1. Move `modules/epoch.gui.rounded_rect.ixx` into `Engine/dep/EpochGui/modules/`.
2. Move `src/rounded_rect.cpp` into `Engine/src/epochgui/`.
3. Add both files to the existing `epoch_gui` target.
4. Feed `RoundedRectMesh::vertices` and its index buffers into the existing EpochGUI backend adapters.
5. Preserve the current rectangular sprite path as the zero-radius fast path.
6. Cache unchanged meshes by size, radii, border width, and segment count.

The native OpenGL hosts and SVG writer are demonstration adapters. The reusable production component is the rounded-rectangle geometry module.
