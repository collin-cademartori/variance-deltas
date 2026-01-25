#!/usr/bin/env bash
set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Installation paths
INSTALL_PREFIX="${HOME}/.local"
APP_DIR="${INSTALL_PREFIX}/share/variance-deltas"
BIN_DIR="${INSTALL_PREFIX}/bin"
SYMLINK_PATH="${BIN_DIR}/vd"

print_section "Variance Deltas Installation Script"

# Check if build directory exists
if [ ! -d "$SCRIPT_DIR/build" ]; then
    print_error "Build directory not found. Please run ./build.sh first."
    exit 1
fi

# Verify build artifacts exist
print_info "Verifying build artifacts..."
MISSING_FILES=()

if [ ! -f "$SCRIPT_DIR/build/ws_server" ]; then
    MISSING_FILES+=("ws_server")
fi

if [ ! -f "$SCRIPT_DIR/build/graph_test" ]; then
    MISSING_FILES+=("graph_test")
fi

if [ ! -f "$SCRIPT_DIR/build/model_parser" ]; then
    MISSING_FILES+=("model_parser")
fi

if [ ! -d "$SCRIPT_DIR/build/client" ]; then
    MISSING_FILES+=("client/")
fi

if [ ${#MISSING_FILES[@]} -ne 0 ]; then
    print_error "Missing build artifacts: ${MISSING_FILES[*]}"
    print_error "Please run ./build.sh first."
    exit 1
fi

print_success "All build artifacts found"

# Remove old installation if it exists
print_section "Removing previous installation (if any)"

if [ -d "$APP_DIR" ]; then
    print_info "Removing old application directory: $APP_DIR"
    rm -rf "$APP_DIR"
    print_success "Removed old application directory"
else
    print_info "No previous application directory found"
fi

if [ -L "$SYMLINK_PATH" ] || [ -f "$SYMLINK_PATH" ]; then
    print_info "Removing old symlink/wrapper: $SYMLINK_PATH"
    rm -f "$SYMLINK_PATH"
    print_success "Removed old symlink/wrapper"
else
    print_info "No previous symlink found"
fi

# Create installation directories
print_section "Creating installation directories"

mkdir -p "$APP_DIR"
print_success "Created application directory: $APP_DIR"

mkdir -p "$BIN_DIR"
print_success "Created bin directory: $BIN_DIR"

# Install application files
print_section "Installing application files"

print_info "Copying ws_server as vd..."
cp "$SCRIPT_DIR/build/ws_server" "$APP_DIR/vd"
chmod +x "$APP_DIR/vd"
print_success "Installed vd (main executable)"

print_info "Copying graph_test..."
cp "$SCRIPT_DIR/build/graph_test" "$APP_DIR/"
chmod +x "$APP_DIR/graph_test"
print_success "Installed graph_test"

print_info "Copying model_parser..."
cp "$SCRIPT_DIR/build/model_parser" "$APP_DIR/"
chmod +x "$APP_DIR/model_parser"
print_success "Installed model_parser"

print_info "Copying client files..."
cp -r "$SCRIPT_DIR/build/client" "$APP_DIR/"
print_success "Installed client files"

# Create symlink
print_section "Creating symlink"

ln -s "$APP_DIR/vd" "$SYMLINK_PATH"
print_success "Created symlink: $SYMLINK_PATH -> $APP_DIR/vd"

# Check if bin directory is in PATH
print_section "Installation complete!"

echo ""
print_info "Installation summary:"
echo "  Application files: $APP_DIR"
echo "  Symlink:           $SYMLINK_PATH"
echo ""

if [[ ":$PATH:" == *":$BIN_DIR:"* ]]; then
    print_success "$BIN_DIR is in your PATH"
    echo ""
    print_info "You can now run: vd -M <model> -D <data> -S <stan> -N <num>"
else
    print_warning "$BIN_DIR is not in your PATH"
    echo ""
    print_info "Add the following line to your ~/.bashrc or ~/.zshrc:"
    echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
    print_info "Then reload your shell or run: source ~/.bashrc"
    echo ""
    print_info "Alternatively, you can run directly:"
    echo "  $APP_DIR/vd -M <model> -D <data> -S <stan> -N <num>"
fi

echo ""
print_success "Installation finished successfully!"
