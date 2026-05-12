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
if [ "$dry_run" -eq 1 ]; then
  echo "DRY_RUN=1: dry run mode, exiting before build/deploy steps"
  exit 0
fi

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

# --------------------- Optional: create a beautified DMG ---------------------
# Set CREATE_DMG=0 to skip DMG creation
if [ "${CREATE_DMG:-1}" != "0" ]; then
  DMG_BASENAME="bookget-macos-arm64"
  if [ -n "${VERSION:-}" ]; then
    DMG_PATH="$BUILD_DIR/${DMG_BASENAME}-${VERSION}.dmg"
  else
    DMG_PATH="$BUILD_DIR/${DMG_BASENAME}.dmg"
  fi
  TMP_DMG="$BUILD_DIR/bookget_tmp.dmg"
  STAGE_DIR="$BUILD_DIR/dmg-stage"
  APP_NAME="bookget.app"
  APP_PATH="$BUILD_DIR/$APP_NAME"
  VOL_NAME="Bookget"

  if [ ! -d "$APP_PATH" ]; then
    echo "Error: $APP_PATH not found, skipping DMG creation."
  else
    echo "Preparing staging area: $STAGE_DIR"
    rm -rf "$STAGE_DIR"
    mkdir -p "$STAGE_DIR/.background"

    # Copy app into staging area
    cp -R "$APP_PATH" "$STAGE_DIR/"

    # Copy project background if exists (repo root screenshot.png)
    REPO_BG="$(pwd)/screenshot.png"
    if [ -f "$REPO_BG" ]; then
      echo "Using background image: $REPO_BG"
      cp "$REPO_BG" "$STAGE_DIR/.background/background.png"
    else
      echo "No screenshot.png found in repo, skipping background image."
    fi

    # Create Applications symlink for easy drag-and-drop
    (cd "$STAGE_DIR" && ln -s /Applications Applications)

    # Ensure writable so Finder shows the proper icon badges
    chmod -R u+w "$STAGE_DIR/$APP_NAME"

    # Create a read-write temporary DMG we can customize
    echo "Creating temporary read-write DMG: $TMP_DMG"
    if command -v hdiutil >/dev/null 2>&1; then
      hdiutil create -volname "$VOL_NAME" -srcfolder "$STAGE_DIR" -ov -format UDRW "$TMP_DMG"
    else
      echo "hdiutil not found; cannot create DMG"
    fi

    # Mount the temporary DMG to a temporary mount point
    MOUNT_DIR=$(mktemp -d /tmp/bookget_dmg_mount.XXXX)
    echo "Mounting $TMP_DMG to $MOUNT_DIR"
    ATTACH_OUT=$(hdiutil attach -readwrite -noverify -noautoopen "$TMP_DMG" -mountpoint "$MOUNT_DIR" 2>&1) || true

    # Run AppleScript to configure Finder window (only when Finder is available)
    if command -v osascript >/dev/null 2>&1; then
      echo "Configuring Finder window layout via AppleScript"

      # Prepare AppleScript. It will:
      # - open the volume
      # - set icon view, window bounds, icon size
      # - set background image if present
      # - position the app and Applications link
      /usr/bin/osascript <<APPLE_SCRIPT
tell application "Finder"
  try
    tell disk "$VOL_NAME"
      open
      set current view of container window to icon view
      set toolbar visible of container window to false
      set statusbar visible of container window to false
      set the bounds of container window to {100, 100, 700, 420}

      set viewOptions to the icon view options of container window
      set arrangement of viewOptions to not arranged
      set icon size of viewOptions to 128

      -- set background image if available
      try
        set background picture of container window to file ".background:background.png"
      end try

      -- position icons (tweak coordinates as needed)
      try
        set position of item "$APP_NAME" of container window to {120, 120}
      end try
      try
        set position of item "Applications" of container window to {420, 120}
      end try

      close
      open
      update without registering applications
    end tell
  end try
end tell
APPLE_SCRIPT

      # Small delay to ensure Finder writes .DS_Store
      sleep 1
    else
      echo "osascript not available; skipping Finder layout customization"
    fi

    # Detach the DMG
    echo "Detaching $MOUNT_DIR"
    hdiutil detach "$MOUNT_DIR" -force || hdiutil detach "$MOUNT_DIR" || true
    rmdir "$MOUNT_DIR" 2>/dev/null || true

    # Convert to compressed UDZO
    echo "Converting temporary DMG to compressed DMG: $DMG_PATH"
    hdiutil convert "$TMP_DMG" -format UDZO -imagekey zlib-level=9 -o "$DMG_PATH"

    # Clean temporary image
    rm -f "$TMP_DMG"

    echo "DMG created: $DMG_PATH"

    # Clean up staging
    rm -rf "$STAGE_DIR"
  fi
else
  echo "CREATE_DMG=0, skipping DMG creation"
fi

