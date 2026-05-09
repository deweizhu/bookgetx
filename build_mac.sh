#!/usr/bin/env bash
set -euo pipefail
# 用法: ./build_mac.sh [Release|Debug] [jobs] [generator]
BUILD_TYPE=${1:-Release}
JOBS=${2:-$(sysctl -n hw.ncpu 2>/dev/null || echo 2)}
GENERATOR=${3:-}
BUILD_DIR=build

echo "Platform: macOS"
echo "Build type: $BUILD_TYPE, jobs: $JOBS, generator: ${GENERATOR:-(default)}"

dry_run=${DRY_RUN:-0}

mkdir -p "$BUILD_DIR"
if [ -n "$GENERATOR" ]; then
  cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
else
  cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

cmake --build "$BUILD_DIR" --parallel "$JOBS"
echo "Build finished: $BUILD_DIR"

# --------------------- Qt deployment (macdeployqt enforced + fallback) ---------------------
if [ "${AUTO_DEPLOY:-1}" = "0" ]; then
  echo "AUTO_DEPLOY=0, skipping Qt deployment"
  exit 0
fi

if ! command -v macdeployqt >/dev/null 2>&1; then
  echo "Error: macdeployqt not found in PATH.\nInstall Qt (or add macdeployqt to PATH) and try again."
  exit 1
fi

# Run macdeployqt to deploy Qt dependencies
echo "Starting Qt6 dependency deployment using macdeployqt..."
macdeployqt "$BUILD_DIR/bookget.app" -verbose=1 || echo "macdeployqt encountered issues. Proceeding with manual fixes."

# Manually add missing frameworks if necessary
QT_BASE=${QTDIR:-${QT_HOME:-"/usr/local/opt/qt"}}
MISSING_FRAMEWORKS=("QtWebChannel.framework")
for FRAMEWORK in "${MISSING_FRAMEWORKS[@]}"; do
  if [ ! -d "$BUILD_DIR/bookget.app/Contents/Frameworks/$FRAMEWORK" ]; then
    echo "Manually adding missing framework: $FRAMEWORK"
    cp -R "$QT_BASE/lib/$FRAMEWORK" "$BUILD_DIR/bookget.app/Contents/Frameworks/"
  fi
done

# Update paths for dependencies using install_name_tool
install_name_tool -change \
  @rpath/QtWebChannel.framework/Versions/A/QtWebChannel \
  @executable_path/../Frameworks/QtWebChannel.framework/Versions/A/QtWebChannel \
  "$BUILD_DIR/bookget.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/A/QtWebEngineCore"

# Resign the .app package
if [ -n "${DEVELOPER_ID:-}" ]; then
  echo "Signing with Developer ID: $DEVELOPER_ID"
  codesign --force --deep --sign "$DEVELOPER_ID" "$BUILD_DIR/bookget.app"
else
  echo "Signing with ad-hoc signature"
  codesign --force --deep --sign - "$BUILD_DIR/bookget.app"
fi

# Verify the signature
codesign --verify --deep --strict "$BUILD_DIR/bookget.app"
echo "Build and deployment completed successfully."
