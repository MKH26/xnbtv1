#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
#  xnbt  –  build.sh
#  Cross-compiles xnbt.so for Android arm64-v8a using the NDK + CMake.
#
#  Prerequisites:
#    - Android NDK r25c+ installed
#    - CMake 3.21+
#    - LeviLamina SDK (set LEVILAMINADIR)
#    - Python3 (for llvm-strip path helper, optional)
#
#  Usage:
#    chmod +x build.sh
#    ./build.sh [release|debug] [arm64-v8a|x86_64]
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

BUILD_TYPE=${1:-release}
ABI=${2:-arm64-v8a}
BUILD_DIR="build/${ABI}-${BUILD_TYPE}"

# ── Locate NDK ────────────────────────────────────────────────────────────────
if [ -z "${ANDROID_NDK_HOME:-}" ]; then
    # Try common locations
    for d in \
        "$HOME/Android/Sdk/ndk/25.2.9519653" \
        "$HOME/android-ndk-r25c" \
        "/opt/android-ndk"; do
        [ -d "$d" ] && ANDROID_NDK_HOME="$d" && break
    done
fi

if [ -z "${ANDROID_NDK_HOME:-}" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set and NDK not found in common paths."
    exit 1
fi

echo "  NDK        : $ANDROID_NDK_HOME"
echo "  ABI        : $ABI"
echo "  Build type : $BUILD_TYPE"
echo "  LeviLamina : ${LEVILAMINADIR:-<not set – stubs will be used>}"
echo ""

CMAKE_BUILD_TYPE="Release"
[ "$BUILD_TYPE" = "debug" ] && CMAKE_BUILD_TYPE="Debug"

TOOLCHAIN="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"

cmake -B "$BUILD_DIR" -S . \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-26" \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    ${LEVILAMINADIR:+-DLEVILAMINADIR="$LEVILAMINADIR"} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "$BUILD_DIR" --parallel "$(nproc)"

OUT="$BUILD_DIR/xnbt.so"
if [ -f "$OUT" ]; then
    SIZE=$(du -sh "$OUT" | cut -f1)
    echo ""
    echo "✓  Built: $OUT  ($SIZE)"
    echo ""
    echo "Deploy to device:"
    echo "  adb push $OUT /data/local/tmp/"
    echo "  # then copy to LeviLamina plugins dir"
fi
