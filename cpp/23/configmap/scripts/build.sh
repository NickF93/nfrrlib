#!/usr/bin/env bash
# Comprehensive build script for configmap project
# Supports: clean, configure, build, install, clang-tidy, cppcheck

set -e  # Exit on error

# Default values
BUILD_TYPE="Debug"
DO_CLEAN=false
DO_CONFIGURE=false
DO_BUILD=false
DO_INSTALL=false
DO_TEST=false
DO_CLANG_TIDY=false
DO_CPPCHECK=false
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
INSTALL_DIR="${PROJECT_ROOT}/install"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build and analysis script for configmap project.

OPTIONS:
    --clean             Clean build/ and install/ directories
    --debug             Configure for Debug build (default)
    --release           Configure for Release build
    --configure         Run CMake configuration
    --build             Build the project
    --test              Run tests (CTest)
    --install           Install to ./install directory
    --clang-tidy        Run clang-tidy static analysis
    --cppcheck          Run cppcheck static analysis
    --all               Do everything: clean, configure, build, test, install
    -j, --jobs N        Number of parallel jobs (default: ${PARALLEL_JOBS})
    -h, --help          Show this help message

EXAMPLES:
    # Clean and build debug version
    $0 --clean --debug --configure --build --install

    # Build release version
    $0 --release --configure --build

    # Run all static analysis
    $0 --clang-tidy --cppcheck

    # Do everything (clean, configure, build, install)
    $0 --all

    # Quick rebuild
    $0 --build --install

EOF
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

clean_directories() {
    log_info "Cleaning build and install directories..."

    if [[ -d "${BUILD_DIR}" ]]; then
        rm -rf "${BUILD_DIR}"
        log_success "Removed ${BUILD_DIR}"
    fi

    if [[ -d "${INSTALL_DIR}" ]]; then
        rm -rf "${INSTALL_DIR}"
        log_success "Removed ${INSTALL_DIR}"
    fi

    log_success "Clean completed"
}

configure_project() {
    log_info "Configuring project with CMake (${BUILD_TYPE})..."

    mkdir -p "${BUILD_DIR}"

    local cmake_args=(
        -S "${PROJECT_ROOT}"
        -B "${BUILD_DIR}"
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
        -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )

    if ! cmake "${cmake_args[@]}"; then
        log_error "CMake configuration failed"
        exit 1
    fi

    log_success "Configuration completed"
}

build_project() {
    log_info "Building project..."

    if [[ ! -d "${BUILD_DIR}" ]]; then
        log_error "Build directory does not exist. Run --configure first."
        exit 1
    fi

    if ! cmake --build "${BUILD_DIR}" --parallel "${PARALLEL_JOBS}"; then
        log_error "Build failed"
        exit 1
    fi

    log_success "Build completed"
}

install_project() {
    log_info "Installing to ${INSTALL_DIR}..."

    if [[ ! -d "${BUILD_DIR}" ]]; then
        log_error "Build directory does not exist. Run --build first."
        exit 1
    fi

    if ! cmake --install "${BUILD_DIR}"; then
        log_error "Installation failed"
        exit 1
    fi

    log_success "Installation completed"
}

run_tests() {
    log_info "Running tests..."

    if [[ ! -d "${BUILD_DIR}" ]]; then
        log_error "Build directory does not exist. Run --build first."
        exit 1
    fi

    cd "${BUILD_DIR}"

    if ! ctest --output-on-failure --parallel "${PARALLEL_JOBS}"; then
        log_error "Tests failed"
        exit 1
    fi

    cd "${PROJECT_ROOT}"
    log_success "All tests passed"
}

run_clang_tidy() {
    log_info "Running clang-tidy..."

    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        log_error "compile_commands.json not found. Run --configure first."
        exit 1
    fi

    local tidy_script="${SCRIPT_DIR}/run_clang_tidy.sh"
    if [[ -x "${tidy_script}" ]]; then
        export BUILD_DIR="${BUILD_DIR}"
        if bash "${tidy_script}"; then
            log_success "clang-tidy completed successfully"
        else
            log_warning "clang-tidy found issues (exit code: $?)"
            return 1
        fi
    else
        log_error "clang-tidy script not found or not executable: ${tidy_script}"
        exit 1
    fi
}

run_cppcheck() {
    log_info "Running cppcheck..."

    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        log_error "compile_commands.json not found. Run --configure first."
        exit 1
    fi

    local cppcheck_script="${SCRIPT_DIR}/run_cppcheck.sh"
    if [[ -x "${cppcheck_script}" ]]; then
        export BUILD_DIR="${BUILD_DIR}"
        if bash "${cppcheck_script}"; then
            log_success "cppcheck completed successfully"
        else
            log_warning "cppcheck found issues (exit code: $?)"
            return 1
        fi
    else
        log_error "cppcheck script not found or not executable: ${cppcheck_script}"
        exit 1
    fi
}

# Parse command-line arguments
if [[ $# -eq 0 ]]; then
    print_usage
    exit 0
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            DO_CLEAN=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            DO_CONFIGURE=true
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            DO_CONFIGURE=true
            shift
            ;;
        --configure)
            DO_CONFIGURE=true
            shift
            ;;
        --build)
            DO_BUILD=true
            shift
            ;;
        --install)
            DO_INSTALL=true
            shift
            ;;
        --test)
            DO_TEST=true
            shift
            ;;
        --clang-tidy)
            DO_CLANG_TIDY=true
            shift
            ;;
        --cppcheck)
            DO_CPPCHECK=true
            shift
            ;;
        --all)
            DO_CLEAN=true
            DO_CONFIGURE=true
            DO_BUILD=true
            DO_TEST=true
            DO_INSTALL=true
            shift
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Execute requested operations in logical order
log_info "Starting build script for configmap project"
log_info "Build type: ${BUILD_TYPE}"
log_info "Project root: ${PROJECT_ROOT}"
log_info "Build directory: ${BUILD_DIR}"
log_info "Install directory: ${INSTALL_DIR}"
echo ""

[[ "${DO_CLEAN}" == true ]] && clean_directories
[[ "${DO_CONFIGURE}" == true ]] && configure_project
[[ "${DO_BUILD}" == true ]] && build_project
[[ "${DO_TEST}" == true ]] && run_tests
[[ "${DO_INSTALL}" == true ]] && install_project
[[ "${DO_CLANG_TIDY}" == true ]] && run_clang_tidy
[[ "${DO_CPPCHECK}" == true ]] && run_cppcheck

echo ""
log_success "All requested operations completed successfully!"
