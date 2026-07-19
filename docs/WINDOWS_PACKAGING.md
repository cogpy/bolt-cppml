# Windows Packaging Guide

This document describes how to build and package **Bolt C++ ML** as a Windows
desktop application: a portable ZIP archive, an NSIS installer, and an MSIX
package — built locally or by CI (see [CI automation](#ci-automation)).

## Overview

The packaging foundation consists of:

| Piece | Location |
|---|---|
| Install rules + CPack config | `CMakeLists.txt` (bottom of file) |
| Windows version/icon resources | `resources/windows/bolt.rc`, `resources/windows/bolt.ico` |
| Icon generator script | `scripts/generate_icon.py` |
| Windows CMake presets | `CMakePresets.json` (`windows-vcpkg`, `windows-vcpkg-release`, `windows-zip`, `windows-nsis`) |
| MSIX manifest + visual assets | `packaging/msix/AppxManifest.xml`, `packaging/msix/Assets/` |
| MSIX asset generator script | `scripts/generate_msix_assets.py` |
| MSIX packaging script | `scripts/package-msix.ps1` |
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

## MSIX packaging

MSIX is the modern Windows app package format: per-user, containerized
installs with clean uninstall, and the submission format for the Microsoft
Store. Bolt ships as a full-trust desktop package (`runFullTrust`).

### Layout

| Piece | Purpose |
|---|---|
| `packaging/msix/AppxManifest.xml` | Package manifest — identity `BoltCppML.IDE`, version `1.0.0.0`, x64, placeholder publisher `CN=BoltCppML Dev` |
| `packaging/msix/Assets/*.png` | Visual assets (Square44x44, StoreLogo 50x50, Square150x150, Wide310x150), generated from the same bolt design as the icon — regenerate with `python scripts/generate_msix_assets.py` |
| `scripts/package-msix.ps1` | Stages, patches, validates, packs and (optionally) signs the package |

The committed manifest declares only `bin\bolt.exe`, which always builds. An
MSIX manifest cannot contain conditional entries, so when the staged install
tree contains the optional GUI (`bin\gui_main.exe`, built only when
ImGui/GLFW/OpenGL are available), `package-msix.ps1` patches a second
`<Application>` entry into the staged manifest and relabels the console entry
*Bolt Console* — matching the NSIS shortcut names.

### Prerequisites

- Everything under [Prerequisites](#prerequisites) above (a built tree or a
  CPack ZIP artifact)
- Windows 10/11 SDK for `makeappx.exe` and `signtool.exe`
  (`winget install Microsoft.WindowsSDK.10.0.22621`, or any SDK ≥ 10.0.17763).
  Without the SDK the script still produces a **validated MSIX layout
  directory** and prints the `makeappx pack` command to run elsewhere.

### Producing the package

```powershell
# From a configured/built CMake build directory (default: build\):
.\scripts\package-msix.ps1

# From a CPack ZIP artifact instead:
.\scripts\package-msix.ps1 -ZipPath build\bolt-cppml-1.0.0-win64.zip

# Pack and sign with a self-signed development certificate:
.\scripts\package-msix.ps1 -Sign

# All options (staging dir, version override, existing cert, ...):
.\scripts\package-msix.ps1 -Help
```

The script stages the `app` install component via
`cmake --install <BuildDir> --prefix <layout> --component app` (or unpacks
the ZIP), copies `AppxManifest.xml` + `Assets\`, patches the manifest,
validates the layout (well-formed manifest, correct namespaces, every
referenced executable and image present), then runs
`makeappx pack /o /d <layout> /p bolt-cppml-<version>-x64.msix`.

### Signing and trusting the development certificate

MSIX packages **must be signed** before they can be installed normally, and
the certificate subject must exactly match the manifest `Publisher`
(`CN=BoltCppML Dev`). `-Sign` creates a matching self-signed certificate in
`Cert:\CurrentUser\My` on demand (the private key stays in the store — never
commit certificates or keys) and signs with
`signtool sign /fd SHA256 /sha1 <thumbprint> <msix>`.

To sideload on a test machine, export the **public** certificate and add it
to the machine's *Trusted People* store (requires an elevated prompt):

```powershell
# On the build machine - export the public .cer (no private key)
$cert = Get-ChildItem Cert:\CurrentUser\My |
    Where-Object Subject -eq 'CN=BoltCppML Dev' | Select-Object -First 1
Export-Certificate -Cert $cert -FilePath bolt-dev.cer

# On the target machine (elevated)
Import-Certificate -FilePath bolt-dev.cer `
    -CertStoreLocation Cert:\LocalMachine\TrustedPeople
```

Remove the trust again with
`Get-ChildItem Cert:\LocalMachine\TrustedPeople | Where-Object Subject -eq 'CN=BoltCppML Dev' | Remove-Item`.

### Installing and uninstalling

```powershell
# Install (per-user; requires the signing cert to be trusted, see above)
Add-AppxPackage .\build\msix\bolt-cppml-1.0.0.0-x64.msix

# Alternative during development (Developer Mode, no signing needed):
# registers the staged layout directly from the layout folder
Add-AppxPackage -Register .\build\msix\layout\AppxManifest.xml

# Uninstall
Get-AppxPackage BoltCppML.IDE | Remove-AppxPackage
```

After install, *Bolt C++ ML* (and *Bolt Console*, when the GUI is packaged)
appear in the Start menu.

### Microsoft Store submission note

The committed identity (`BoltCppML.IDE` / `CN=BoltCppML Dev`) is a
**development placeholder**. For a Store submission, reserve the app name in
[Partner Center](https://partner.microsoft.com/dashboard), then replace
`Identity/@Name`, `Identity/@Publisher` and
`Properties/PublisherDisplayName` in `packaging/msix/AppxManifest.xml` with
the exact values shown under *Product identity*. Store packages are signed
by Microsoft during ingestion, so no self-signed certificate is involved.

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
contains a commented TODO hook where `scripts/package-msix.ps1` (see
[MSIX packaging](#msix-packaging)) will slot in as a follow-up.
