#!/usr/bin/env bash
# Run cppcheck on all project source files

set -e

# Determine build directory
BUILD_DIR="${BUILD_DIR:-build}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    echo "Error: compile_commands.json not found in ${BUILD_DIR}"
    echo "Please configure the project first with CMake."
    exit 1
fi

echo "Running cppcheck on project sources..."
cppcheck \
    --project="${BUILD_DIR}/compile_commands.json" \
    --enable=warning,style,performance,portability \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    --relative-paths \
    --template='{file}:{line}:{column}: warning: [cppcheck/{severity}] {message}' \
    --quiet

echo "cppcheck completed successfully"
