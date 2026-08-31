#!/usr/bin/env bash
#
# Builds INQNET_GUI on Linux without Qt Creator.
#
# Usage:
#   ./scripts/build-linux.sh                    # release build
#   ./scripts/build-linux.sh --config debug
#   ./scripts/build-linux.sh --qmake ~/Qt/6.11.1/gcc_64/bin/qmake
#   ./scripts/build-linux.sh --timetagger-inc /opt/timetagger/include
#
set -euo pipefail

CONFIG=release
QMAKE_BIN=""
TIMETAGGER_INC=""
BUILD_DIR=""
JOBS="$(nproc 2>/dev/null || echo 4)"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
pro_file="$repo_root/source/INQNET_GUI.pro"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config)          CONFIG="$2"; shift 2 ;;
        --qmake)           QMAKE_BIN="$2"; shift 2 ;;
        --timetagger-inc)  TIMETAGGER_INC="$2"; shift 2 ;;
        --build-dir)       BUILD_DIR="$2"; shift 2 ;;
        -j|--jobs)         JOBS="$2"; shift 2 ;;
        -h|--help)         sed -n '2,12p' "$0"; exit 0 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

[[ "$CONFIG" == "release" || "$CONFIG" == "debug" ]] || {
    echo "--config must be 'release' or 'debug'" >&2; exit 1; }

[[ -n "$BUILD_DIR" ]] || BUILD_DIR="$repo_root/build/linux-$CONFIG"

# ---- Locate qmake ----
if [[ -z "$QMAKE_BIN" ]]; then
    for candidate in qmake6 qmake; do
        if command -v "$candidate" >/dev/null 2>&1; then
            QMAKE_BIN="$(command -v "$candidate")"
            break
        fi
    done
fi
[[ -n "$QMAKE_BIN" ]] || {
    echo "ERROR: no qmake on PATH. Install Qt 6 and/or pass --qmake <path>." >&2
    echo "       See README.md for the Qt Online Installer steps." >&2
    exit 1; }

echo "qmake:  $QMAKE_BIN"
"$QMAKE_BIN" --version

# ---- Check the Qt SerialPort module is present (ovdl.cpp needs it) ----
qt_prefix="$("$QMAKE_BIN" -query QT_INSTALL_PREFIX)"
if [[ ! -f "$qt_prefix/mkspecs/modules/qt_lib_serialport.pri" ]]; then
    echo "ERROR: Qt SerialPort module missing from $qt_prefix." >&2
    echo "       Add it via the Qt Maintenance Tool (Additional Libraries ->" >&2
    echo "       Qt Serial Port), or install qt6-serialport-dev." >&2
    exit 1
fi

# ---- runtime_data/databaseInfo.json is gitignored, but the .pro's copydata
#      step needs it, so a fresh clone would otherwise fail the build ----
if [[ ! -f "$repo_root/runtime_data/databaseInfo.json" ]]; then
    cp "$repo_root/runtime_data/databaseInfo.json.example" \
       "$repo_root/runtime_data/databaseInfo.json"
    echo "NOTE: created runtime_data/databaseInfo.json from the template --"
    echo "      fill in your real MySQL credentials before connecting."
fi

# ---- Build ----
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

qmake_args=("$pro_file" "CONFIG+=$CONFIG")
[[ -n "$TIMETAGGER_INC" ]] && qmake_args+=("TIMETAGGER_INC=$TIMETAGGER_INC")

echo
echo "=== qmake ==="
"$QMAKE_BIN" "${qmake_args[@]}"

echo
echo "=== compiling (-j$JOBS) ==="
make "-j$JOBS"

echo
echo "Built: $BUILD_DIR/PROGRAM"
echo "Run it from $BUILD_DIR (the app opens its config files by relative path)."
