# Windows Packaging Guide

This document describes how to build and package **Bolt C++ ML** as a Windows
desktop application: a portable ZIP archive and an NSIS installer. MSIX
packaging and CI automation are planned follow-ups (see the placeholder
sections at the end).

## Overview

The packaging foundation consists of:

| Piece | Location |
|---|---|
| Install rules + CPack config | `CMakeLists.txt` (bottom of file) |
| Windows version/icon resources | `resources/windows/bolt.rc`, `resources/windows/bolt.ico` |
| Icon generator script | `scripts/generate_icon.py` |
| Windows CMake presets | `CMakePresets.json` (`windows-vcpkg`, `windows-vcpkg-release`, `windows-zip`, `windows-nsis`) |

Only the main application is packaged (the `app` install component):

```
bin/bolt.exe          # console application
bin/gui_main.exe      # desktop GUI (when ImGui/GLFW/OpenGL are available)
bin/*.dll             # dependent runtime DLLs (shared builds)
share/bolt/LICENSE
share/bolt/README.md
```

Demo and test executables are **not** installed or packaged.

## Prerequisites

- Windows 10/11 x64
- CMake ≥ 3.25 (presets schema v6; the project itself needs ≥ 3.15, and
  runtime-DLL installation needs ≥ 3.21)
- Visual Studio 2019/2022 with the C++ workload (or another supported
  toolchain, e.g. MinGW-w64)
- [vcpkg](https://github.com/microsoft/vcpkg) with the `VCPKG_ROOT`
  environment variable set (for the full-featured GUI build)
- Optional: [NSIS](https://nsis.sourceforge.io/) (`makensis` on `PATH`) for
  the installer

## Building

### With vcpkg (recommended, full features)

```powershell
# Configure (installs manifest dependencies: curl, jsoncpp, glfw3, imgui, opengl)
cmake --preset windows-vcpkg

# Build the application targets
cmake --build --preset windows-vcpkg-release
```

The `windows-vcpkg` preset uses the `x64-windows` triplet and the toolchain
file from `$env:VCPKG_ROOT`. `gui_main` is only built when ImGui is found;
`bolt` always builds.

### Minimal build (no external dependencies)

All third-party dependencies are optional. For a console-only package:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_LLAMA_CPP=OFF
cmake --build build --target bolt
```

## Packaging

### ZIP (portable archive)

```powershell
# With presets:
cpack --preset windows-zip

# Or directly from the build directory:
cd build\windows-vcpkg
cpack -G ZIP
```

Produces `bolt-cppml-1.0.0-<system>.zip` containing `bin/` and `share/bolt/`.

### NSIS installer

Requires NSIS to be installed (`winget install NSIS.NSIS`).

```powershell
# With presets:
cpack --preset windows-nsis

# Or directly:
cd build\windows-vcpkg
cpack -G NSIS
```

The installer:

- installs under `$PROGRAMFILES64\Bolt C++ ML`
- shows the MIT license (`LICENSE`) during setup
- creates Start Menu shortcuts for **Bolt C++ ML** (`gui_main.exe`) and
  **Bolt Console** (`bolt.exe`)
- uses the application icon from `resources/windows/bolt.ico`
- registers an uninstaller (uninstall-before-reinstall enabled)

## Desktop integration details

- `gui_main` is built with `WIN32_EXECUTABLE TRUE` on Windows, so launching
  it does not open a console window. Since `gui_main.cpp` uses a standard
  `main()`, MSVC builds link with `/ENTRY:mainCRTStartup`.
- Both `bolt.exe` and `gui_main.exe` embed `resources/windows/bolt.rc`:
  a VERSIONINFO block (1.0.0.0, product name *Bolt C++ ML*, MIT license
  notice) and the application icon.
- The icon is committed at `resources/windows/bolt.ico` (16/32/48/256 px).
  Regenerate it with `python scripts/generate_icon.py` (stdlib only).

## Runtime DLL deployment

On Windows, `install(FILES $<TARGET_RUNTIME_DLLS:...>)` copies dependent
shared-library DLLs (e.g. from vcpkg `x64-windows` dynamic triplet) into
`bin/` at install/package time, so the packaged tree runs standalone. When
building with vcpkg's applocal deployment, DLLs are also copied next to the
build outputs automatically.

## MSIX packaging (planned)

> **Placeholder** — an upcoming PR adds MSIX packaging on top of this
> foundation: an `AppxManifest.xml`, asset generation from `bolt.ico`
> imagery, `makeappx`/`signtool` invocation scripts, and (optionally) a
> `CPACK` external step. Until then, use the ZIP or NSIS artifacts.

## CI automation (planned)

> **Placeholder** — an upcoming PR adds a GitHub Actions workflow that
> configures with the `windows-vcpkg` preset, builds `bolt` + `gui_main`,
> runs `cpack` for ZIP (and NSIS), and uploads the artifacts on tags.
