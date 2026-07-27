# EpochGUI Rounded Corners

Standalone C++23 prototype for backend-neutral rounded rectangle geometry using EpochGUI's existing `Vec2` and `Rect` types.

The repository is intentionally separate from EpochEngine. It does not modify the editor, renderer frame order, GUI sprite batching, or any production EpochGUI source.

## Windows release

The Windows x64 release contains:

- `EpochGuiRoundedCorners.exe` — native Direct2D window displaying all rounded-corner cases
- `epoch_gui_rounded_corners_demo.exe` — headless geometry validator and SVG generator
- `epochgui_rounded_corners_demo.svg` — generated reference output
- `README.md`

The Windows application uses the same `RoundedRectMesh` geometry as the headless contract. It is not a separate hard-coded drawing path.

## What it demonstrates

- Uniform and independent per-corner radii
- Pill-shaped controls
- Filled and bordered rounded rectangles
- CSS-style proportional radius normalization when adjacent radii overlap
- Bounded tessellation from 1 to 64 segments per corner
- Sanitized non-finite and negative input
- Deterministic vertex and triangle-index generation
- Native Direct2D presentation on Windows
- A headless SVG output for renderer-independent inspection and automated validation

## Requirements for source builds

- CMake 3.28 or newer
- A C++23 compiler with module support
- Ninja or Visual Studio generator
- A local EpochEngine checkout

The project imports the real `epoch.gui` module from:

```text
<EPOCH_ENGINE_ROOT>/dep/EpochGui/modules/epoch.gui.ixx
```

`EPOCH_ENGINE_ROOT` is the path to the `Engine` directory inside EpochEngine.

## Build

When both repositories are sibling directories, the EpochEngine path is detected automatically:

```text
Projects/
|-- EpochEngine/
|   `-- Engine/
`-- EpochGuiRoundedCorners/
```

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
.\build\EpochGuiRoundedCorners.exe
```

For any other layout, pass the engine path explicitly:

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DEPOCH_ENGINE_ROOT="C:/Code/EpochEngine/Engine"
```

The headless executable writes `epochgui_rounded_corners_demo.svg`. An alternate output path may be supplied as its first argument.

## Production integration

After the geometry and visual result are accepted:

1. Move `modules/epoch.gui.rounded_rect.ixx` into `Engine/dep/EpochGui/modules/`.
2. Move `src/rounded_rect.cpp` into `Engine/src/epochgui/`.
3. Add both files to the existing `epoch_gui` CMake target.
4. Re-export the module from the intended EpochGUI public surface if desired.
5. Feed `RoundedRectMesh::vertices` and the selected index buffer into each backend's existing GUI adapter.
6. Preserve the current rectangular sprite path as the zero-radius fast path.
7. Cache unchanged meshes by size, radii, border width, and segment count.

The Direct2D and SVG presenters are demonstration adapters. The reusable production component is the backend-neutral rounded-rectangle module and mesh generator.
