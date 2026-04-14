#!/usr/bin/env bash
# Setup script for building bucklespring on macOS
# This handles the alure dependency which is no longer available in Homebrew

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAC_PREFIX="$SCRIPT_DIR/mac"
ALURE_VERSION="1.2"
ALURE_ARCHIVE="alure-${ALURE_VERSION}.tar.bz2"
ALURE_URL="http://web.archive.org/web/20190529213651/https://kcat.strangesoft.net/alure-releases/${ALURE_ARCHIVE}"

# Detect Homebrew prefix
if command -v brew &>/dev/null; then
    BREW_PREFIX="$(brew --prefix)"
else
    echo "Error: Homebrew not found. Please install it from https://brew.sh"
    exit 1
fi

echo "==> Installing Homebrew dependencies..."
brew install openal-soft cmake pkg-config

OPENAL_PREFIX="$(brew --prefix openal-soft)"

echo "==> Creating build directories..."
mkdir -p "$MAC_PREFIX"/{lib/pkgconfig,include,build}

# Download alure if needed
if [[ ! -f "$MAC_PREFIX/build/$ALURE_ARCHIVE" ]]; then
    echo "==> Downloading alure from archive.org..."
    curl -L -o "$MAC_PREFIX/build/$ALURE_ARCHIVE" "$ALURE_URL"
fi

# Extract and build alure
echo "==> Extracting alure..."
cd "$MAC_PREFIX/build"
tar -xf "$ALURE_ARCHIVE"

echo "==> Building alure..."
cd "alure-${ALURE_VERSION}/build"

# Configure alure to use openal-soft instead of system OpenAL
# Note: -DCMAKE_POLICY_VERSION_MINIMUM=3.5 is needed for newer cmake versions
cmake .. \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$MAC_PREFIX" \
    -DCMAKE_PREFIX_PATH="$OPENAL_PREFIX" \
    -DOPENAL_INCLUDE_DIR="$OPENAL_PREFIX/include" \
    -DOPENAL_LIBRARY="$OPENAL_PREFIX/lib/libopenal.dylib" \
    -DBUILD_STATIC=OFF \
    -DBUILD_EXAMPLES=OFF

make -j"$(sysctl -n hw.ncpu)"
make install

echo "==> Creating pkg-config files..."

# Ensure AL include directory exists for alure.h
mkdir -p "$MAC_PREFIX/include/AL"
cp "$MAC_PREFIX/include/OpenAL/alure.h" "$MAC_PREFIX/include/AL/"

# Create openal.pc pointing to openal-soft
cat > "$MAC_PREFIX/lib/pkgconfig/openal.pc" << EOF
prefix=$OPENAL_PREFIX
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: OpenAL
Description: OpenAL (Open Audio Library) software implementation
Version: 1.24.0
Libs: -L\${libdir} -lopenal
Cflags: -I\${includedir}
EOF

# Create alure.pc pointing to our local build
cat > "$MAC_PREFIX/lib/pkgconfig/alure.pc" << EOF
prefix=$MAC_PREFIX
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: alure
Description: Audio Library Tools REloaded
Version: ${ALURE_VERSION}
Requires: openal
Libs: -L\${libdir} -lalure
Cflags: -I\${includedir}
EOF

echo "==> Setup complete!"
echo ""
echo "You can now build bucklespring with:"
echo "  make clean && make"
echo ""
echo "Note: You may need to run with sudo for key capture:"
echo "  sudo ./buckle"
echo ""
echo "Also grant Accessibility permissions to Terminal in:"
echo "  System Preferences → Security & Privacy → Privacy → Accessibility"
