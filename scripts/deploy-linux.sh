#!/usr/bin/env bash
#
# Assembles a self-contained INQNET_GUI directory on Linux, so the target
# machine needs neither Qt nor Qt Creator installed.
#
# The result is dist/INQNET_GUI/ containing:
#   PROGRAM            the executable
#   lib/               Qt + vendor shared libraries it links against
#   plugins/           Qt plugins (platform, imageformats, sqldrivers, ...)
#   qt.conf            points Qt at plugins/ instead of the build machine's Qt
#   INQNET_GUI         launcher script -- run THIS, not PROGRAM directly
#   *.conf, *.json     runtime config, read by relative path
#
# Usage:
#   ./scripts/deploy-linux.sh                            # uses build/linux-release
#   ./scripts/deploy-linux.sh --exe build/linux-debug/PROGRAM
#
# NOTE: read the licensing section in README.md before redistributing --
# the vendor libraries are proprietary and not yours to hand out freely.
#
set -euo pipefail

EXE=""
OUT_DIR=""
QMAKE_BIN=""

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --exe)     EXE="$2"; shift 2 ;;
        --out)     OUT_DIR="$2"; shift 2 ;;
        --qmake)   QMAKE_BIN="$2"; shift 2 ;;
        -h|--help) sed -n '2,20p' "$0"; exit 0 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

[[ -n "$EXE" ]]     || EXE="$repo_root/build/linux-release/PROGRAM"
[[ -n "$OUT_DIR" ]] || OUT_DIR="$repo_root/dist/INQNET_GUI"

[[ -f "$EXE" ]] || { echo "ERROR: executable not found: $EXE" >&2; exit 1; }

if [[ -z "$QMAKE_BIN" ]]; then
    for candidate in qmake6 qmake; do
        command -v "$candidate" >/dev/null 2>&1 && { QMAKE_BIN="$(command -v "$candidate")"; break; }
    done
fi
[[ -n "$QMAKE_BIN" ]] || { echo "ERROR: no qmake on PATH; pass --qmake." >&2; exit 1; }

qt_plugins="$("$QMAKE_BIN" -query QT_INSTALL_PLUGINS)"

mkdir -p "$OUT_DIR/lib" "$OUT_DIR/plugins"
cp "$EXE" "$OUT_DIR/PROGRAM"

# ---- Shared libraries ----
# Walk ldd output and copy everything that isn't part of the base system.
# Filtering by path keeps glibc/libstdc++/X11/OpenGL from the host, which is
# what you want: bundling glibc across distros causes more problems than it
# solves.
echo "=== shared libraries ==="
skip_re='^(linux-vdso|/lib(64)?/ld-|libc\.so|libm\.so|libdl\.so|libpthread\.so|librt\.so|libstdc\+\+\.so|libgcc_s\.so|libGL|libEGL|libX11|libxcb|libwayland|libdrm|libgbm|libglib|libgobject|libgio|libfontconfig|libfreetype|libz\.so|libssl|libcrypto)'

copy_deps() {
    local target="$1"
    ldd "$target" 2>/dev/null | awk '{ for (i=1;i<=NF;i++) if ($i ~ /^\//) { print $i; break } }' | \
    while read -r dep; do
        [[ -f "$dep" ]] || continue
        local base; base="$(basename "$dep")"
        if [[ "$base" =~ $skip_re || "$dep" =~ $skip_re ]]; then continue; fi
        if [[ ! -f "$OUT_DIR/lib/$base" ]]; then
            cp -L "$dep" "$OUT_DIR/lib/"
            echo "  $base"
            copy_deps "$dep"          # transitive: Qt libs pull in more Qt libs
        fi
    done
}
copy_deps "$OUT_DIR/PROGRAM"

# ---- Qt plugins ----
# qwayland-* is included alongside xcb so the bundle works on either session.
echo "=== Qt plugins ==="
for group in platforms imageformats iconengines sqldrivers tls networkinformation platformthemes; do
    if [[ -d "$qt_plugins/$group" ]]; then
        mkdir -p "$OUT_DIR/plugins/$group"
        cp -L "$qt_plugins/$group"/*.so "$OUT_DIR/plugins/$group/" 2>/dev/null || true
        echo "  $group"
        for so in "$OUT_DIR/plugins/$group"/*.so; do
            [[ -f "$so" ]] && copy_deps "$so"
        done
    fi
done

# ---- qt.conf ----
# Without this, Qt looks for plugins at the build machine's absolute Qt path
# and the app dies with "could not find or load the Qt platform plugin xcb".
cat > "$OUT_DIR/qt.conf" <<'EOF'
[Paths]
Prefix = .
Plugins = plugins
Libraries = lib
EOF

# ---- Vendor libraries ----
echo "=== vendor libraries ==="
for so in "$repo_root"/lib/*.so*; do
    [[ -f "$so" ]] && { cp -L "$so" "$OUT_DIR/lib/"; echo "  $(basename "$so")"; }
done

# ---- Runtime config ----
echo "=== runtime config ==="
for f in "$repo_root"/runtime_data/*; do
    case "$f" in *.example) continue ;; esac
    [[ -f "$f" ]] && { cp "$f" "$OUT_DIR/"; echo "  $(basename "$f")"; }
done

# ---- Launcher ----
# Sets LD_LIBRARY_PATH to the bundled lib/ and cd's into the bundle, because
# the app opens its config files by bare relative path.
cat > "$OUT_DIR/INQNET_GUI" <<'EOF'
#!/usr/bin/env bash
here="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"
export LD_LIBRARY_PATH="$here/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$here/plugins"
cd "$here"
exec ./PROGRAM "$@"
EOF
chmod +x "$OUT_DIR/INQNET_GUI" "$OUT_DIR/PROGRAM"

size="$(du -sh "$OUT_DIR" | cut -f1)"
echo
echo "Bundle: $OUT_DIR ($size)"
echo "Launch with: $OUT_DIR/INQNET_GUI"
