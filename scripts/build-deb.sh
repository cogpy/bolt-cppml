#!/usr/bin/env bash
# Builds the bolt-cppml .deb from an existing shared-library build tree.
#
# Usage: scripts/build-deb.sh <build-dir> <version> <output.deb>
#
# Used by the Debian and APT packaging workflows; it can be run locally too:
#   cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
#   cmake --build build
#   scripts/build-deb.sh build 1.1.0 bolt-cppml_1.1.0_amd64.deb

set -euo pipefail

if [ "$#" -ne 3 ]; then
    echo "usage: $0 <build-dir> <version> <output.deb>" >&2
    exit 2
fi

BUILD_DIR="$(cd "$1" && pwd)"
VERSION="$2"
OUTPUT="$3"
SOURCE_DIR="$(cd "$(dirname "$0")/.." && pwd)"

PACKAGE_NAME="bolt-cppml"
ARCH="$(dpkg --print-architecture)"
LIB_DIR="usr/lib/$PACKAGE_NAME"

if [ ! -x "$BUILD_DIR/bolt" ]; then
    echo "error: $BUILD_DIR/bolt not found; build the project first" >&2
    exit 1
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
PKG="$WORK/pkg"

install -d "$PKG/DEBIAN" "$PKG/usr/bin" "$PKG/$LIB_DIR" "$PKG/etc/ld.so.conf.d" \
    "$PKG/usr/share/applications" "$PKG/usr/share/doc/$PACKAGE_NAME" "$PKG/usr/share/pixmaps"

install -m 755 "$BUILD_DIR/bolt" "$PKG/usr/bin/bolt"

# Shared libraries: bolt's own and the bundled llama.cpp ones. cp -a keeps the
# libfoo.so -> libfoo.so.1 symlinks instead of copying the file three times.
lib_count=0
for lib in "$BUILD_DIR"/libbolt_lib.so* "$BUILD_DIR"/bin/*.so*; do
    [ -e "$lib" ] || continue
    cp -a "$lib" "$PKG/$LIB_DIR/"
    lib_count=$((lib_count + 1))
done
if [ "$lib_count" -eq 0 ]; then
    echo "error: no shared libraries found in $BUILD_DIR; configure with -DBUILD_SHARED_LIBS=ON" >&2
    exit 1
fi
find "$PKG/$LIB_DIR" -type f -exec chmod 644 {} +

# The libraries live in a private directory; tell the dynamic linker about it.
echo "/$LIB_DIR" > "$PKG/etc/ld.so.conf.d/$PACKAGE_NAME.conf"

# Runtime dependencies, worked out from what the binaries actually link so the
# list is right on every Ubuntu release. dpkg-shlibdeps wants a debian/control
# file in the current directory.
mkdir -p "$WORK/debian"
touch "$WORK/debian/control"
mapfile -t elf_files < <(find "$PKG/usr/bin" "$PKG/$LIB_DIR" -type f)
DEPENDS="$(cd "$WORK" && dpkg-shlibdeps -O --ignore-missing-info -l"$PKG/$LIB_DIR" "${elf_files[@]}" 2>/dev/null |
    sed -n 's/^shlibs:Depends=//p')"
if [ -z "$DEPENDS" ]; then
    echo "error: could not work out the package dependencies" >&2
    exit 1
fi

INSTALLED_SIZE="$(du -sk "$PKG" | cut -f1)"

# Continuation lines of Description must start with a space; a blank line is
# written as " .".
cat > "$PKG/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: devel
Priority: optional
Architecture: $ARCH
Installed-Size: $INSTALLED_SIZE
Depends: $DEPENDS
Maintainer: Bolt C++ Team <team@bolt-cppml.org>
Homepage: https://github.com/cogpy/bolt-cppml
Description: Modern C++ implementation of Bolt IDE with AI integration
 Bolt C++ is a modern C++ implementation of the Bolt IDE core components
 with AI integration and real-time collaborative features.
 .
 Features include:
  - GGML integration for AI model inference
  - RWKV implementation for RNN capabilities
  - Multi-cursor support and syntax highlighting
  - Integrated debugger with breakpoints
  - WebSocket server for real-time collaboration
  - Comprehensive ImGui dark-mode interface
EOF

for script in postinst postrm; do
    printf '#!/bin/sh\nset -e\nldconfig\n' > "$PKG/DEBIAN/$script"
    chmod 755 "$PKG/DEBIAN/$script"
done

# bolt is a terminal program, so the launcher opens it in a terminal.
cat > "$PKG/usr/share/applications/$PACKAGE_NAME.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Bolt C++
Comment=Modern C++ IDE with AI integration
Exec=/usr/bin/bolt
Icon=$PACKAGE_NAME
Terminal=true
Categories=Development;IDE;
EOF

install -m 644 "$SOURCE_DIR/README.md" "$PKG/usr/share/doc/$PACKAGE_NAME/README.md"
if [ -f "$SOURCE_DIR/LICENSE" ]; then
    install -m 644 "$SOURCE_DIR/LICENSE" "$PKG/usr/share/doc/$PACKAGE_NAME/copyright"
fi
if [ -f "$SOURCE_DIR/generated-icon.png" ]; then
    install -m 644 "$SOURCE_DIR/generated-icon.png" "$PKG/usr/share/pixmaps/$PACKAGE_NAME.png"
fi

dpkg-deb --root-owner-group --build "$PKG" "$OUTPUT"
dpkg-deb --info "$OUTPUT"
