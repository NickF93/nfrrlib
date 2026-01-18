#!/usr/bin/env bash
# Run clang-tidy on all project source files

set -e

# Determine build directory
BUILD_DIR="${BUILD_DIR:-build}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    echo "Error: compile_commands.json not found in ${BUILD_DIR}"
    echo "Please configure the project first with CMake."
    exit 1
fi

# Find all C++ source files in src/ and tests/
SOURCE_FILES=$(find examples tests -type f \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \))

if [[ -z "${SOURCE_FILES}" ]]; then
    echo "No source files found"
    exit 0
fi

echo "Running clang-tidy on project sources..."
echo "${SOURCE_FILES}" | xargs clang-tidy -p "${BUILD_DIR}" \
    --extra-arg=-std=gnu++23 \
    --extra-arg=-stdlib=libstdc++

echo "clang-tidy completed successfully"
