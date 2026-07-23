#!/bin/bash

# Navidrome Export Tool - Build Script

set -e

echo "=== Building Navidrome Export Tool ==="

# Check dependencies
echo "Checking dependencies..."
if ! command -v cmake &> /dev/null; then
    echo "cmake not found. Install with:"
    echo "   Ubuntu/Debian: sudo apt-get install cmake"
    echo "   Fedora/RHEL:   sudo dnf install cmake"
    echo "   Arch:          sudo pacman -S cmake"
    exit 1
fi

if ! command -v curl-config &> /dev/null; then
    echo "libcurl not found. Install with:"
    echo "   Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev"
    echo "   Fedora/RHEL:   sudo dnf install libcurl-devel"
    echo "   Arch:          sudo pacman -S curl"
    exit 1
fi

echo "All dependencies found"

# Create build directory
echo "Setting up build directory..."
if [ ! -d "build" ]; then
    mkdir -p build

    cd build

    # Configure
    echo "Configuring CMake..."
    cmake ..
else
    cd build

    # Configure
    echo "Configuring CMake..."
    cmake ..

    # Clean build directory
    echo "Cleaning building directory..."
    make clean
fi

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "=== navidrome-export Build Complete ==="
echo "Executable: $(pwd)/navidrome-export"
echo ""
