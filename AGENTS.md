# AGENTS.md

## Cursor Cloud specific instructions

`coco` is a single C++17 desktop application: a Qt6 + OpenCASCADE (OCCT) 3D
CAD/hull viewer. It builds with CMake + Ninja. There is no test suite, no
backend, and no network — "running the app" means launching the GUI and
verifying the OCCT viewport renders. See `README`-less repo docs in
`CMakeLists.txt`, `CMakePresets.json`, `flake.nix`, and `vcpkg.json` for the
canonical build definitions.

### Dependencies (already installed via the startup update script)

System dependencies (Qt6 6.4, OCCT 7.6, build tools, Mesa/X11, clang-format,
cmake-format) come from apt and are refreshed by the update script.

Non-obvious caveat — VTK: `CMakeLists.txt` requires VTK with the
`GUISupportQt` component, but apt's `libvtk9-qt-dev` is built against **Qt5**,
which hard-conflicts with this Qt6 app (`INTERFACE_QT_MAJOR_VERSION` error at
configure). The repo's `flake.nix`/`vcpkg.json` solve this by building VTK with
Qt6, but Nix/vcpkg are not used in this environment. Instead, a minimal **VTK
9.3.1 built with `VTK_QT_VERSION=6`** is installed to `/usr/local` (persisted in
the VM snapshot). VTK is not referenced anywhere in `src/` — it is only a
build/link dependency — so the exact VTK version does not matter functionally.
Do **not** reinstall `libvtk9-qt-dev` (it re-introduces the Qt5 conflict). If
`/usr/local/lib/cmake/vtk-9.3` is ever missing, rebuild VTK from source with
`-DVTK_QT_VERSION=6`, `-DVTK_GROUP_ENABLE_Qt=DONT_WANT`,
`-DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES` and `install` to `/usr/local`.

### Build

Two required extra flags beyond the presets:
- `-DCMAKE_CXX_COMPILER=g++` — the default `c++` resolves to clang here, which
  fails to link (`cannot find -lstdc++`). The project (and `flake.nix`) use gcc.
- `-DCMAKE_PREFIX_PATH=/usr/local` — so CMake finds the Qt6 VTK in `/usr/local`
  instead of any system VTK.

```
cmake --preset linux-debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_PREFIX_PATH=/usr/local
cmake --build build/linux-debug
```

Binary: `build/linux-debug/coco`. (Presets `linux-release`/`ninja-vcpkg-*` also
exist; see `CMakePresets.json`.)

### Run (headless)

The app is a GUI that forces `QT_QPA_PLATFORM=xcb` and needs an X server plus an
OpenGL context. In this headless VM use Xvfb + Mesa software GL:

```
Xvfb :99 -screen 0 1280x1024x24 +extension GLX +render -noreset &
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 build/linux-debug/coco
```

On launch it displays a demo box (`BRepPrimAPI_MakeBox`). Viewport controls
(from `src/app/ViewportController.cpp`): left-drag = rotate, wheel = zoom,
`S` = shaded, `W` = wireframe, `T`/`B`/`L`/`R` = view presets, `F` = fit.
`QStandardPaths: XDG_RUNTIME_DIR not set` on stdout is harmless.

### Lint / format

`cmake-format --check CMakeLists.txt` works. `clang-format` is installed, but
clang-format 18 fails to parse the repo `.clang-format` because of its trailing
`...` YAML document-end marker (`unknown key '...'`); the config is otherwise
valid and the sources are already formatted. This is a repo/tool-version quirk —
do not modify `.clang-format` as part of unrelated work.
