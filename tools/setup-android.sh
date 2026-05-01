#!/bin/bash
#
# setup-android.sh — Configure Android build environment for Supercell Wx
#
# Usage: ./tools/setup-android.sh [BUILD_DIR] [VENV_PATH] [ANDROID_ABI]
#
# Arguments:
#   BUILD_DIR       Build directory (default: build-android-${ANDROID_ABI})
#   VENV_PATH       Python virtual environment path (default: .venv)
#   ANDROID_ABI     Android ABI to build for (default: arm64-v8a)
#                   Options: arm64-v8a, armeabi-v7a, x86_64
#
# Environment Variables:
#   ANDROID_SDK_ROOT    Path to Android SDK (required)
#   ANDROID_NDK_HOME    Path to Android NDK (required)
#   JAVA_HOME           Path to Java JDK (required)
#
# Prerequisites:
#   - Android SDK and NDK installed
#   - Java JDK 11+ installed
#   - Qt 6.11.0 for Android (install via aqtinstall or Qt Maintenance Tool)
#   - Conan 2.x and CMake 3.24+
#

set -euo pipefail

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

# Parameters
BUILD_DIR="${1:-build-android-${ANDROID_ABI:-arm64-v8a}}"
VENV_PATH="${2:-$REPO_ROOT/.venv}"
ANDROID_ABI="${3:-arm64-v8a}"

# Expand paths
BUILD_DIR="$(cd "$REPO_ROOT" && mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR" && pwd)"
VENV_PATH="$(cd "$REPO_ROOT" && python3 -c "import os; print(os.path.abspath('$VENV_PATH'))")"

echo "=========================================="
echo "Supercell Wx — Android Build Setup"
echo "=========================================="
echo ""
echo "Build Directory: $BUILD_DIR"
echo "Python Venv:     $VENV_PATH"
echo "Android ABI:     $ANDROID_ABI"
echo ""

# Validate required environment variables
if [[ -z "${ANDROID_SDK_ROOT:-}" ]]; then
    echo "ERROR: ANDROID_SDK_ROOT not set"
    echo "Please export ANDROID_SDK_ROOT=/path/to/sdk"
    exit 1
fi

if [[ -z "${ANDROID_NDK_HOME:-}" ]]; then
    echo "ERROR: ANDROID_NDK_HOME not set"
    echo "Please export ANDROID_NDK_HOME=/path/to/ndk"
    exit 1
fi

if [[ -z "${JAVA_HOME:-}" ]]; then
    echo "ERROR: JAVA_HOME not set"
    echo "Please export JAVA_HOME=/path/to/jdk"
    exit 1
fi

echo "Validated environment variables:"
echo "  ANDROID_SDK_ROOT:  $ANDROID_SDK_ROOT"
echo "  ANDROID_NDK_HOME:  $ANDROID_NDK_HOME"
echo "  JAVA_HOME:         $JAVA_HOME"
echo ""

# Validate Android NDK toolchain exists
NDK_TOOLCHAIN="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
if [[ ! -f "$NDK_TOOLCHAIN" ]]; then
    echo "ERROR: Android NDK toolchain not found: $NDK_TOOLCHAIN"
    exit 1
fi
echo "✓ Found Android NDK toolchain: $NDK_TOOLCHAIN"
echo ""

# Activate or create Python virtual environment
if [[ ! -d "$VENV_PATH" ]]; then
    echo "Creating Python virtual environment at: $VENV_PATH"
    python3 -m venv "$VENV_PATH"
fi

echo "Activating Python virtual environment..."
source "$VENV_PATH/bin/activate"

# Install Conan profile
echo ""
echo "Installing Conan Android profiles..."
PROFILE_DIR="$REPO_ROOT/tools/conan/profiles"

# Determine API level mapping for ABI
case "$ANDROID_ABI" in
    arm64-v8a)
        ARCH="armv8"
        PROFILE_NAME="scwx-android_arm64-v8a"
        PROFILE_NAME_DEBUG="scwx-android_arm64-v8a-debug"
        ;;
    armeabi-v7a)
        ARCH="armv7"
        PROFILE_NAME="scwx-android_armeabi-v7a"
        PROFILE_NAME_DEBUG="scwx-android_armeabi-v7a-debug"
        ;;
    x86_64)
        ARCH="x86_64"
        PROFILE_NAME="scwx-android_x86_64"
        PROFILE_NAME_DEBUG="scwx-android_x86_64-debug"
        ;;
    *)
        echo "ERROR: Unknown Android ABI: $ANDROID_ABI"
        echo "Supported: arm64-v8a, armeabi-v7a, x86_64"
        exit 1
        ;;
esac

conan config install "$PROFILE_DIR" -tf profiles || true
echo "✓ Conan profiles installed"
echo ""

# Show next steps
echo "=========================================="
echo "Next Steps"
echo "=========================================="
echo ""
echo "1. Make sure Qt 6.11.0 for Android is installed."
echo "   Install via aqtinstall:"
echo ""
echo "   pip install aqtinstall"
echo "   aqt install-qt android android 6.11.0 android_armv8 -m qtimageformats qtmultimedia"
echo ""
echo "2. Configure CMake for $ANDROID_ABI:"
echo ""
echo "   cd $BUILD_DIR"
echo "   cmake .. -G Ninja \\"
echo "     -DCMAKE_TOOLCHAIN_FILE=\$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \\"
echo "     -DANDROID_ABI=$ANDROID_ABI \\"
echo "     -DANDROID_PLATFORM=android-33 \\"
echo "     -DCMAKE_BUILD_TYPE=Release \\"
echo "     -DCONAN_HOST_PROFILE=$PROFILE_NAME \\"
echo "     -DCONAN_BUILD_PROFILE=$PROFILE_NAME"
echo ""
echo "   Or use CMake presets (preferred):"
echo ""
echo "   cmake --preset android-${ANDROID_ABI}-release"
echo ""
echo "3. Build the application:"
echo ""
echo "   cmake --build $BUILD_DIR --target supercell-wx"
echo ""
echo "4. Create APK/AAB with Qt deployment tools:"
echo ""
echo "   androiddeployqt --input $BUILD_DIR/AndroidDeploymentSettings.json \\"
echo "                   --output $BUILD_DIR/android-build \\"
echo "                   --android-platform android-33 \\"
echo "                   --apk-dir $BUILD_DIR"
echo ""
echo "=========================================="
echo "Setup complete!"
echo "=========================================="
