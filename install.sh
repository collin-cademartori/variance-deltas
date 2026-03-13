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

# Parse arguments
INSTALL_PREFIX="${HOME}/.local"
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --prefix=*)
            INSTALL_PREFIX="${1#*=}"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--prefix <path>]"
            echo ""
            echo "Install variance-deltas to the specified prefix (default: ~/.local)"
            echo ""
            echo "Options:"
            echo "  --prefix <path>  Installation prefix (default: \$HOME/.local)"
            echo "                   App files go to <prefix>/share/variance-deltas/"
            echo "                   Symlink goes to <prefix>/bin/vd"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Usage: $0 [--prefix <path>]"
            exit 1
            ;;
    esac
done

# Get script directory (where the archive was extracted)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Installation paths
APP_DIR="${INSTALL_PREFIX}/share/variance-deltas"
BIN_DIR="${INSTALL_PREFIX}/bin"
SYMLINK_PATH="${BIN_DIR}/vd"

print_section "Variance Deltas Installation Script"

print_info "Install prefix: $INSTALL_PREFIX"
print_info "Application directory: $APP_DIR"
print_info "Symlink: $SYMLINK_PATH"

# Verify build artifacts exist (look in script's own directory)
print_section "Verifying build artifacts"
MISSING_FILES=()

for file in vd vd-backend vd-model-parser ranger; do
    if [ ! -f "$SCRIPT_DIR/$file" ]; then
        MISSING_FILES+=("$file")
    fi
done

if [ ! -d "$SCRIPT_DIR/client" ]; then
    MISSING_FILES+=("client/")
fi

if [ ${#MISSING_FILES[@]} -ne 0 ]; then
    print_error "Missing files: ${MISSING_FILES[*]}"
    print_error "Make sure you are running this script from the extracted archive directory."
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

print_info "Copying vd..."
cp "$SCRIPT_DIR/vd" "$APP_DIR/"
chmod +x "$APP_DIR/vd"
print_success "Installed vd (main executable)"

print_info "Copying vd-backend..."
cp "$SCRIPT_DIR/vd-backend" "$APP_DIR/"
chmod +x "$APP_DIR/vd-backend"
print_success "Installed vd-backend"

print_info "Copying vd-model-parser..."
cp "$SCRIPT_DIR/vd-model-parser" "$APP_DIR/"
chmod +x "$APP_DIR/vd-model-parser"
print_success "Installed vd-model-parser"

print_info "Copying ranger..."
cp "$SCRIPT_DIR/ranger" "$APP_DIR/"
chmod +x "$APP_DIR/ranger"
print_success "Installed ranger"

print_info "Copying client files..."
cp -r "$SCRIPT_DIR/client" "$APP_DIR/"
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
    echo "  export PATH=\"${BIN_DIR}:\$PATH\""
    echo ""
    print_info "Then reload your shell or run: source ~/.bashrc"
    echo ""
    print_info "Alternatively, you can run directly:"
    echo "  $APP_DIR/vd -M <model> -D <data> -S <stan> -N <num>"
fi

echo ""
print_success "Installation finished successfully!"
