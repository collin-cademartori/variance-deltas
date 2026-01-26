#!/usr/bin/env bash
set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_error() {
    echo -e "${RED}ERROR: $1${NC}" >&2
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_info() {
    echo -e "${BLUE}→ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_section() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

print_section "Variance Deltas Build Script"

# ============================================================================
# Phase 1: Tool Detection
# ============================================================================
print_section "Phase 1: Checking for required build tools"

MISSING_TOOLS=()

check_tool() {
    local tool=$1
    local install_msg=$2

    if command -v "$tool" &> /dev/null; then
        print_success "$tool found ($(command -v $tool))"
        return 0
    else
        print_error "$tool not found"
        print_warning "$install_msg"
        MISSING_TOOLS+=("$tool")
        return 1
    fi
}

check_tool "opam" "Install opam: https://opam.ocaml.org/doc/Install.html"

# Set up opam environment so dune and other OCaml tools are in PATH
print_info "Setting up opam environment..."
eval $(opam env)
print_success "opam environment configured"

check_tool "dune" "Install dune: opam install dune"
check_tool "cmake" "Install cmake: brew install cmake (macOS) or apt install cmake (Linux)"
check_tool "make" "Install make: Usually pre-installed on Unix systems"
check_tool "deno" "Install deno: curl -fsSL https://deno.land/install.sh | sh"

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    print_error "Missing required tools: ${MISSING_TOOLS[*]}"
    print_error "Please install the missing tools and try again."
    exit 1
fi

print_success "All required build tools found!"

# ============================================================================
# Phase 1.5: C++ Library Dependency Check
# ============================================================================
print_section "Phase 1.5: Checking for C++ library dependencies"

MISSING_CPP_LIBS=()

check_cpp_lib() {
    local lib_name=$1
    local pkg_config_name=$2
    local install_msg=$3

    # Try pkg-config first
    if command -v pkg-config &> /dev/null && pkg-config --exists "$pkg_config_name" 2>/dev/null; then
        local version=$(pkg-config --modversion "$pkg_config_name" 2>/dev/null || echo "unknown")
        print_success "$lib_name found via pkg-config (version: $version)"
    else
        # If no pkg-config, we'll let CMake handle it but warn the user
        print_warning "$lib_name not found via pkg-config (CMake will attempt to find it)"
        print_warning "$install_msg"
        MISSING_CPP_LIBS+=("$lib_name")
    fi

    # Always return 0 - this is informational only, CMake does the real check
    return 0
}

# Check for pkg-config itself
if ! command -v pkg-config &> /dev/null; then
    print_warning "pkg-config not found - skipping library version checks"
    print_warning "Install pkg-config for better dependency verification: brew install pkg-config (macOS) or apt install pkg-config (Linux)"
    print_warning "CMake will still attempt to find required libraries..."
else
    # Check each C++ dependency
    check_cpp_lib "Boost" "boost" "Install: brew install boost (macOS) or apt install libboost-all-dev (Linux)"
    check_cpp_lib "Eigen3" "eigen3" "Install: brew install eigen (macOS) or apt install libeigen3-dev (Linux)"
    check_cpp_lib "OpenSSL" "openssl" "Install: brew install openssl (macOS) or apt install libssl-dev (Linux)"
    check_cpp_lib "zlib" "zlib" "Install: brew install zlib (macOS) or apt install zlib1g-dev (Linux)"

    # nlohmann_json check (package name varies)
    if pkg-config --exists nlohmann_json 2>/dev/null; then
        local version=$(pkg-config --modversion nlohmann_json 2>/dev/null || echo "unknown")
        print_success "nlohmann_json found via pkg-config (version: $version)"
    elif [ -f "/usr/include/nlohmann/json.hpp" ] || [ -f "/usr/local/include/nlohmann/json.hpp" ] || [ -f "/opt/homebrew/include/nlohmann/json.hpp" ]; then
        print_success "nlohmann_json found (header detected)"
    else
        print_warning "nlohmann_json not found via pkg-config or common paths"
        print_warning "Install: brew install nlohmann-json (macOS) or apt install nlohmann-json3-dev (Linux)"
        MISSING_CPP_LIBS+=("nlohmann_json")
    fi
fi

if [ ${#MISSING_CPP_LIBS[@]} -ne 0 ]; then
    print_warning "Some C++ libraries could not be verified: ${MISSING_CPP_LIBS[*]}"
    print_warning "CMake will attempt to locate them. If the build fails, install the missing libraries."
    echo ""
fi

print_success "C++ dependency check completed"

# ============================================================================
# Phase 2: Dependency Installation
# ============================================================================
print_section "Phase 2: Installing package dependencies"

# fg_parser dependencies
print_info "Installing fg_parser (OCaml) dependencies..."
cd "$SCRIPT_DIR/fg_parser"
if opam install . --deps-only -y; then
    print_success "fg_parser dependencies installed"
else
    print_error "Failed to install fg_parser dependencies"
    exit 1
fi

# tree_frontend dependencies
print_info "Installing tree_frontend (npm) dependencies via Deno..."
cd "$SCRIPT_DIR/tree_frontend"
# Deno reads package.json and installs all dependencies to node_modules/
if deno install --node-modules-dir; then
    print_success "tree_frontend dependencies installed"
else
    print_error "Failed to install tree_frontend dependencies"
    exit 1
fi

# ws_server dependencies (if deno.json has dependencies)
print_info "Installing ws_server (deno) dependencies..."
cd "$SCRIPT_DIR/ws_server"
# Deno automatically downloads dependencies on first run, but we can cache them
if deno cache main.ts; then
    print_success "ws_server dependencies cached"
else
    print_warning "Failed to cache ws_server dependencies (may still work)"
fi

cd "$SCRIPT_DIR"

# ============================================================================
# Phase 3: Clean Phase
# ============================================================================
print_section "Phase 3: Cleaning previous build artifacts"

print_info "Removing top-level build directory..."
rm -rf "$SCRIPT_DIR/build/"
print_success "Removed build/"

print_info "Cleaning fg_parser..."
cd "$SCRIPT_DIR/fg_parser"
dune clean 2>/dev/null || true
print_success "Cleaned fg_parser"

print_info "Cleaning graph_backend..."
cd "$SCRIPT_DIR/graph_backend"
rm -rf build/
print_success "Cleaned graph_backend"

print_info "Cleaning tree_frontend..."
cd "$SCRIPT_DIR/tree_frontend"
rm -rf build/
print_success "Cleaned tree_frontend"

print_info "Cleaning ws_server..."
cd "$SCRIPT_DIR/ws_server"
rm -rf build/
print_success "Cleaned ws_server"

print_info "Cleaning ranger..."
cd "$SCRIPT_DIR/ranger/cpp_version"
rm -rf build/
print_success "Cleaned ranger"

cd "$SCRIPT_DIR"
print_success "All clean phases completed"

# ============================================================================
# Phase 4: Build Phase
# ============================================================================
print_section "Phase 4: Building all components"

# Build fg_parser
print_info "Building fg_parser (OCaml)..."
cd "$SCRIPT_DIR/fg_parser"
# Re-evaluate opam environment to pick up local switch if present
eval $(opam env)
if dune build; then
    print_success "fg_parser built successfully"
else
    print_error "Failed to build fg_parser"
    exit 1
fi

# Build graph_backend
print_info "Building graph_backend (C++)..."
cd "$SCRIPT_DIR/graph_backend"
mkdir -p build
cd build
if cmake -DCMAKE_BUILD_TYPE=Release ../src; then
    print_success "CMake configuration completed (Release mode)"
else
    print_error "CMake configuration failed"
    print_error "Make sure C++ dependencies are installed (see warnings above)"
    exit 1
fi

if make; then
    print_success "graph_backend built successfully"
else
    print_error "Failed to build graph_backend"
    exit 1
fi

# Build ranger (random forest CLI)
print_info "Building ranger (C++)..."
cd "$SCRIPT_DIR/ranger/cpp_version"
mkdir -p build
cd build
if cmake -DCMAKE_BUILD_TYPE=Release ..; then
    print_success "ranger CMake configuration completed"
else
    print_error "ranger CMake configuration failed"
    exit 1
fi

if make; then
    print_success "ranger built successfully"
else
    print_error "Failed to build ranger"
    exit 1
fi

# Build tree_frontend
print_info "Building tree_frontend (Svelte/TypeScript)..."
cd "$SCRIPT_DIR/tree_frontend"
if deno run -A npm:vite build; then
    print_success "tree_frontend built successfully"
else
    print_error "Failed to build tree_frontend"
    exit 1
fi

# Build ws_server
print_info "Building ws_server (Deno)..."
cd "$SCRIPT_DIR/ws_server"
mkdir -p build
if deno compile --allow-net --allow-read --allow-run --output build/ws_server main.ts; then
    print_success "ws_server built successfully"
else
    print_error "Failed to build ws_server"
    exit 1
fi

cd "$SCRIPT_DIR"
print_success "All components built successfully"

# ============================================================================
# Phase 5: Assembly Phase
# ============================================================================
print_section "Phase 5: Assembling build directory"

print_info "Creating build directory structure..."
mkdir -p "$SCRIPT_DIR/build/client"

print_info "Copying model_parser executable..."
if cp "$SCRIPT_DIR/fg_parser/_build/default/bin/model_parser.exe" "$SCRIPT_DIR/build/model_parser"; then
    print_success "Copied model_parser (stripped .exe extension)"
else
    print_error "Failed to copy model_parser.exe"
    exit 1
fi

print_info "Copying graph_backend executable..."
if cp "$SCRIPT_DIR/graph_backend/build/graph_test" "$SCRIPT_DIR/build/"; then
    print_success "Copied graph_test"
else
    print_error "Failed to copy graph_test"
    exit 1
fi

print_info "Copying ranger executable..."
if cp "$SCRIPT_DIR/ranger/cpp_version/build/ranger" "$SCRIPT_DIR/build/"; then
    print_success "Copied ranger"
else
    print_error "Failed to copy ranger"
    exit 1
fi

print_info "Copying ws_server executable..."
if cp "$SCRIPT_DIR/ws_server/build/ws_server" "$SCRIPT_DIR/build/"; then
    print_success "Copied ws_server"
else
    print_error "Failed to copy ws_server"
    exit 1
fi

print_info "Copying tree_frontend static files..."
if cp -r "$SCRIPT_DIR/tree_frontend/build/"* "$SCRIPT_DIR/build/client/"; then
    print_success "Copied frontend files"
else
    print_error "Failed to copy frontend files"
    exit 1
fi

print_success "Build directory assembled"

# ============================================================================
# Phase 6: Verification Phase
# ============================================================================
print_section "Phase 6: Verifying build outputs"

VERIFICATION_FAILED=false

verify_file() {
    local file=$1
    local description=$2

    if [ -f "$file" ]; then
        print_success "$description exists"
        return 0
    else
        print_error "$description missing"
        VERIFICATION_FAILED=true
        return 1
    fi
}

verify_executable() {
    local file=$1
    local description=$2

    if [ -f "$file" ] && [ -x "$file" ]; then
        print_success "$description exists and is executable"
        return 0
    elif [ -f "$file" ]; then
        print_error "$description exists but is not executable"
        VERIFICATION_FAILED=true
        return 1
    else
        print_error "$description missing"
        VERIFICATION_FAILED=true
        return 1
    fi
}

verify_executable "$SCRIPT_DIR/build/ws_server" "ws_server"
verify_executable "$SCRIPT_DIR/build/graph_test" "graph_test"
verify_executable "$SCRIPT_DIR/build/model_parser" "model_parser"
verify_executable "$SCRIPT_DIR/build/ranger" "ranger"
verify_file "$SCRIPT_DIR/build/client/index.html" "Frontend index.html"
verify_file "$SCRIPT_DIR/build/client/_app/version.json" "Frontend app bundle"

if [ "$VERIFICATION_FAILED" = true ]; then
    print_error "Build verification failed"
    exit 1
fi

print_success "All build outputs verified!"

# ============================================================================
# Build Complete
# ============================================================================
print_section "Build completed successfully!"

echo ""
print_info "Build artifacts location: $SCRIPT_DIR/build/"
echo ""
print_info "To run the application:"
echo "  cd build"
echo "  ./ws_server -M <model file> -D <data file> -S <stan file prefix> -N <number of chains> [--port <port>]"
echo ""
print_info "Example:"
echo "  cd build"
echo "  ./ws_server -M ../data/model_spec -D ../data/data.json -S ../data/posterior_samples -N 4"
echo ""
print_info "The server defaults to port 8765. Specify --port to use a different port."
echo ""
print_success "Build script finished!"
