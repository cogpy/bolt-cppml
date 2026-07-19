# Windows Packaging Guide

This document describes how to build and package **Bolt C++ ML** as a Windows
desktop application: a portable ZIP archive and an NSIS installer, built
locally or by CI (see [CI automation](#ci-automation)). MSIX packaging is a
planned follow-up (see the placeholder section at the end).

## Overview

The packaging foundation consists of:

| Piece | Location |
|---|---|
| Install rules + CPack config | `CMakeLists.txt` (bottom of file) |
| Windows version/icon resources | `resources/windows/bolt.rc`, `resources/windows/bolt.ico` |
| Icon generator script | `scripts/generate_icon.py` |
| Windows CMake presets | `CMakePresets.json` (`windows-vcpkg`, `windows-vcpkg-release`, `windows-zip`, `windows-nsis`) |
| Packaging CI | `.github/workflows/windows-package.yml`, `.github/workflows/chocolatey-package.yml` |

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

## CI automation

The **Windows Package** workflow (`.github/workflows/windows-package.yml`)
builds the desktop packaging artifacts on `windows-latest`:

1. Loads the MSVC developer environment (the presets use Ninja) and points
   `VCPKG_ROOT` at the runner's preinstalled vcpkg (falling back to a clone
   pinned at the `builtin-baseline` from `vcpkg.json`). vcpkg binary caching
   is persisted through `actions/cache`, so the imgui/glfw/curl dependency
   set only builds from source on a cold cache.
2. Configures with the `windows-vcpkg` preset. `BUILD_SHARED_LIBS` is forced
   `OFF` on the command line because the vendored llama.cpp CMake defaults it
   to `ON` before the root project sets its own default — which would turn
   `bolt_lib` into a DLL that exports no symbols and break all linking.
3. Builds only the packaged targets: `bolt` + `gui_main` via the
   `windows-vcpkg-release` preset when `gui_main` was generated, or `bolt`
   alone when ImGui wasn't found (surfaced in the job summary, not a failure).
4. Runs `cpack --preset windows-zip` and `cpack --preset windows-nsis`
   (`makensis` is ensured first: preinstalled, or installed via Chocolatey).
5. Sanity-checks that `bin/bolt.exe` exists inside the ZIP, then writes a job
   summary (artifact names, sizes, whether `gui_main` was included).

**Triggers** — the workflow runs on:

- `workflow_dispatch` (manual): *Actions → Windows Package → Run workflow*,
  or `gh workflow run windows-package.yml`
- tag pushes matching `v*`
- pull requests that touch packaging-relevant paths (`CMakeLists.txt`,
  `CMakePresets.json`, `vcpkg.json`, `resources/**`, or the workflow itself)

**Artifacts** — every run uploads two artifacts, downloadable from the run
page or with `gh run download <run-id>`:

- `bolt-cppml-windows-zip` — the portable ZIP (`bolt-cppml-<version>-win64.zip`)
- `bolt-cppml-windows-nsis` — the NSIS installer (`bolt-cppml-<version>-win64.exe`)

**Releases** — on a `v*` tag push, a follow-up job attaches both files to the
GitHub release for that tag (creating the release if it doesn't exist yet).

The **Chocolatey Package Build** workflow
(`.github/workflows/chocolatey-package.yml`) reuses the same
preset-plus-CPack pipeline, stages the Chocolatey payload from the CPack
ZIP's `bin/` contents (executables + runtime DLLs), and packs a `.nupkg`
versioned from the tag (leading `v` stripped; the CPack version is used for
non-tag runs). It runs on `v*` tags and manual dispatch, uploads the
`.nupkg` as an artifact, attaches it to the release, and optionally pushes
to chocolatey.org when a `CHOCO_API_KEY` secret is configured.

MSIX packaging is not wired into CI yet — the Windows Package workflow
contains a commented TODO hook where `scripts/package-msix.ps1` will slot in
once the MSIX packaging PR lands.
