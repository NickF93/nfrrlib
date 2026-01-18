# nfrrconfig - C++23 ConfigMap Project

A header-only C++23 library for configuration value management with allocator support.

## Build Requirements

- CMake 3.26 or later
- C++23 compatible compiler:
  - GCC 13+
  - Clang 18+
- clangd for IDE support (recommended)
- clang-tidy for static analysis (optional)
- cppcheck for additional static analysis (optional)

## Building

### Quick Start

```bash
# Configure with Release build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install

# Build
cmake --build build --parallel

# Run tests
./build/test_configmap
./build/configmap_tests

# Install
cmake --install build --prefix ./install
```

### Build Options

- `NFRRCONFIG_USE_GNU_EXTENSIONS`: Use `-std=gnu++23` instead of `-std=c++23` (default: OFF)
- `BUILD_TESTING`: Enable/disable tests (default: ON)

## IDE Setup

### VS Code

The project is configured for VS Code with the following extensions (see [.vscode/extensions.json](.vscode/extensions.json)):

- **llvm-vs-code-extensions.vscode-clangd** - Primary C++ language server
- **ms-vscode.cmake-tools** - CMake integration
- **cppchecksolutionsab.cppcheck-official** - Static analysis

### Important Configuration Files

- `.clangd` - clangd language server configuration
- `.clang-format` - Code formatting rules (LLVM-based)
- `.clang-tidy` - Static analysis checks
- `.vscode/settings.json` - VS Code workspace settings
- `.vscode/tasks.json` - Build and analysis tasks

### Using the Development Tools

The project includes several VS Code tasks accessible via `Ctrl+Shift+B`:

- **analysis:active-file** - Run clang-tidy + cppcheck on current file
- **analysis:project** - Run static analysis on entire project
- **cmake:build:debug** - Configure + build Debug preset
- **cmake:build:release** - Configure + build Release preset

You can also use the provided scripts:

```bash
# Run clang-tidy on all source files
BUILD_DIR=build ./scripts/run_clang_tidy.sh

# Run cppcheck on all source files
BUILD_DIR=build ./scripts/run_cppcheck.sh
```

## Project Structure

```text
.
├── CMakeLists.txt           # Main CMake configuration
├── include/                 # Header files
│   └── nfrrconfig/
│       ├── nfrrconfig.hpp   # Main public header
│       └── impl/            # Implementation details
├── examples/                # Example usage
│   └── main.cpp
├── tests/                   # Unit tests
│   └── test_configmap.cpp
├── cmake/                   # CMake modules
├── scripts/                 # Helper scripts
├── build/                   # Build directory (generated)
└── install/                 # Install directory (generated)
```

## IntelliSense / Language Server

The project is configured to use **clangd** instead of the default C++ IntelliSense engine, as clangd has better C++23 support.

If you see spurious errors about "unknown type name 'concept'" or other C++23 features:

1. Ensure clangd extension is installed
2. Reload the VS Code window (Ctrl+Shift+P → "Developer: Reload Window")
3. Wait for clangd to index the project (check status bar)
4. The `compile_commands.json` is automatically generated in the build directory

The clangd configuration includes:

- Background indexing for fast navigation
- Integrated clang-tidy checks
- Header insertion with include-what-you-use
- Inlay hints for parameter names and deduced types

## Static Analysis

### clang-tidy

Configured in [.clang-tidy](.clang-tidy) with comprehensive checks including:

- Bugprone patterns
- Modernization suggestions
- Performance improvements
- Readability enhancements
- C++ Core Guidelines

Some overly strict checks are disabled to reduce noise.

### cppcheck

Runs with the following checks enabled:

- warning
- style
- performance
- portability

## License

See project documentation for license information.
