#!/bin/bash
set -e

VERSION="1.0.0"
ARCH="amd64"
PACKAGE="navidrome-export"
MAINTAINER="Ryan Name <54684156+Ryan-Diazz@users.noreply.github.com>"

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
STAGING="$ROOT_DIR/${PACKAGE}_${VERSION}_${ARCH}"
BINARY="$ROOT_DIR/build/navidrome-export"

echo "=== Building navidrome-export .deb package ==="

# Build the binary first
echo "Building binary..."
cd "$ROOT_DIR"
bash scripts/build.sh

if [ ! -f "$BINARY" ]; then
    echo "Error: binary not found at $BINARY"
    exit 1
fi

# Clean previous staging dir
rm -rf "$STAGING"

# Create layout
mkdir -p "$STAGING/DEBIAN"
mkdir -p "$STAGING/usr/local/bin"

# Copy binary
cp "$BINARY" "$STAGING/usr/local/bin/$PACKAGE"
chmod 755 "$STAGING/usr/local/bin/$PACKAGE"

# Write control file
cat > "$STAGING/DEBIAN/control" << EOF
Package: $PACKAGE
Version: $VERSION
Architecture: $ARCH
Maintainer: $MAINTAINER
Depends: libcurl4 (>= 7.58)
Description: Export Navidrome music library to CSV/TXT
 A C++ CLI tool to export your Navidrome music library songs to CSV or TXT
 format. Supports Navidrome 0.45.0 and later.
EOF

# Build the .deb
echo "Creating .deb package..."
dpkg-deb --build "$STAGING"

echo "Cleaning Build Artifacts..."
rm -rf "$STAGING"

echo ""
echo "=== Package created: ${PACKAGE}_${VERSION}_${ARCH}.deb ==="
echo "Install with: sudo dpkg -i ${PACKAGE}_${VERSION}_${ARCH}.deb"
