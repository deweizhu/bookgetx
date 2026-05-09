#!/usr/bin/env bash
set -euo pipefail
# 用法: ./build_linux.sh [Release|Debug] [jobs] [generator]
BUILD_TYPE=${1:-Release}
JOBS=${2:-$(nproc 2>/dev/null || echo 2)}
GENERATOR=${3:-}
BUILD_DIR=build

echo "Platform: Linux"
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

# --------------------- Qt deployment (linuxdeployqt enforced) ---------------------
if [ "${AUTO_DEPLOY:-1}" = "0" ]; then
  echo "AUTO_DEPLOY=0, skipping Qt deployment"
  exit 0
fi

if ! command -v linuxdeployqt >/dev/null 2>&1; then
  echo "Error: linuxdeployqt not found in PATH.\nInstall linuxdeployqt and try again."
  exit 1
fi

echo "Starting Qt6 dependency deployment using linuxdeployqt..."

# prefer Release subdir for multi-config generators
SEARCH_DIR="$BUILD_DIR"
if [ -d "$BUILD_DIR/Release" ]; then
  SEARCH_DIR="$BUILD_DIR/Release"
fi

# find executables
exes=()
while IFS= read -r -d '' exe; do
  exes+=("$exe")
done < <(find "$SEARCH_DIR" -type f -perm -111 -not -path "*/CMakeFiles/*" -not -name "*.so" -print0 2>/dev/null || true)

if [ ${#exes[@]} -eq 0 ]; then
  echo "No executables found in $SEARCH_DIR to run linuxdeployqt on. Exiting deploy step."
  exit 0
fi

for exe in "${exes[@]}"; do
  echo "Running linuxdeployqt on $exe"
  if [ "$dry_run" = "1" ]; then
    echo "DRY RUN: linuxdeployqt \"$exe\" -qmake '$QTDIR/bin/qmake' -verbose=1"
  else
    if [ -n "${QTDIR:-}" ]; then
      linuxdeployqt "$exe" -qmake "$QTDIR/bin/qmake" -verbose=1 || echo "linuxdeployqt returned non-zero for $exe"
    else
      linuxdeployqt "$exe" -verbose=1 || echo "linuxdeployqt returned non-zero for $exe"
    fi
  fi
done

echo "linuxdeployqt deployment finished."
