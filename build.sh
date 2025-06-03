#!/bin/bash

# AI Generated Transport Network 2 - Build Script
# Automated build script for the transportation simulation project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project configuration
PROJECT_NAME="ai_gen_transport_2"
BUILD_DIR="build"
CMAKE_BUILD_TYPE="Release"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show help
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -d, --debug         Build in Debug mode (default: Release)"
    echo "  -r, --release       Build in Release mode"
    echo "  -j, --jobs N        Use N parallel jobs for compilation (default: auto-detect)"
    echo "  -v, --verbose       Enable verbose build output"
    echo "  --install           Install the built executable"
    echo "  --test              Run tests after building (if available)"
    echo ""
    echo "Examples:"
    echo "  $0                  # Standard release build"
    echo "  $0 -c -d            # Clean debug build"
    echo "  $0 -j 4 -v          # Build with 4 jobs and verbose output"
    echo "  $0 --clean --test   # Clean build and run tests"
}

# Default values
CLEAN_BUILD=false
DEBUG_BUILD=false
VERBOSE_BUILD=false
INSTALL_BUILD=false
RUN_TESTS=false
JOBS=$(nproc 2>/dev/null || echo "4")

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -d|--debug)
            DEBUG_BUILD=true
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            DEBUG_BUILD=false
            CMAKE_BUILD_TYPE="Release"
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE_BUILD=true
            shift
            ;;
        --install)
            INSTALL_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Print build configuration
print_status "Building ${PROJECT_NAME}"
print_status "Build type: ${CMAKE_BUILD_TYPE}"
print_status "Parallel jobs: ${JOBS}"

# Check for required tools
print_status "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed or not in PATH"
    exit 1
fi

if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
    print_error "Neither make nor ninja build system found"
    exit 1
fi

# Check for optional tools
if command -v dot &> /dev/null; then
    print_status "Graphviz found - DOT files can be visualized"
    GRAPHVIZ_AVAILABLE=true
else
    print_warning "Graphviz not found - DOT visualization will not be available"
    GRAPHVIZ_AVAILABLE=false
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_warning "Build directory doesn't exist, nothing to clean"
    fi
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring project with CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

if [ "$VERBOSE_BUILD" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

# Try to use Ninja if available, otherwise use Make
if command -v ninja &> /dev/null; then
    CMAKE_ARGS+=(-GNinja)
    BUILD_TOOL="ninja"
    print_status "Using Ninja build system"
else
    BUILD_TOOL="make"
    print_status "Using Make build system"
fi

if ! cmake "${CMAKE_ARGS[@]}" ..; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "CMake configuration completed"

# Build the project
print_status "Building project..."
BUILD_ARGS=()

if [ "$BUILD_TOOL" = "ninja" ]; then
    BUILD_ARGS+=(-j "$JOBS")
    if [ "$VERBOSE_BUILD" = true ]; then
        BUILD_ARGS+=(-v)
    fi
else
    BUILD_ARGS+=(-j "$JOBS")
    if [ "$VERBOSE_BUILD" = true ]; then
        BUILD_ARGS+=(VERBOSE=1)
    fi
fi

if ! $BUILD_TOOL "${BUILD_ARGS[@]}"; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Install if requested
if [ "$INSTALL_BUILD" = true ]; then
    print_status "Installing project..."
    if ! $BUILD_TOOL install; then
        print_error "Installation failed"
        exit 1
    fi
    print_success "Installation completed"
fi

# Run tests if requested and available
if [ "$RUN_TESTS" = true ]; then
    print_status "Running tests
