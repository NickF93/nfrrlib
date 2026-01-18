#!/usr/bin/env bash
# Clean build artifacts

set -e

echo "Cleaning build artifacts..."

# Remove build directory
if [[ -d "build" ]]; then
    rm -rf build
    echo "  - Removed build/"
fi

# Remove install directory
if [[ -d "install" ]]; then
    rm -rf install
    echo "  - Removed install/"
fi

# Remove any stray clangd cache
if [[ -d ".cache" ]]; then
    rm -rf .cache
    echo "  - Removed .cache/"
fi

if [[ -d ".clangd" ]]; then
    rm -rf .clangd
    echo "  - Removed .clangd/"
fi

echo "Clean complete!"
