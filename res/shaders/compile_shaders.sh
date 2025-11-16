#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# compile_shaders.sh
# Walk the "code" directory and compile shader files with glslc.
# Input files are expected as name.stage (e.g. myshader.vert)
# Output files will be placed under CODE_DIR/../compiled preserving tree
#
# Usage: ./compile_shaders.sh [CODE_DIR]
# Default CODE_DIR is "$(dirname "$0")/../../code"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODE_DIR="${1:-code}"

# Ensure glslc is available
if ! command -v glslc >/dev/null 2>&1; then
    echo "glslc not found in PATH. Install Vulkan SDK or glslc." >&2
    exit 2
fi

# Where to put compiled shaders (../compiled relative to CODE_DIR)
OUT_BASE="$(cd "$CODE_DIR/.." && pwd)/compiled"

# Allowed shader extensions (stage)
EXTS=("vert" "frag" "comp" "geom" "tesc" "tese" "mesh" "task" "rgen" "rahit" "rchit" "anyhit" "closesthit")

# Build find -iname patterns
find_args=()
for e in "${EXTS[@]}"; do
    find_args+=(-iname "*.${e}")
    find_args+=(-o)
done
# remove trailing -o
unset 'find_args[${#find_args[@]}-1]'

# Discover shader files and compile
if [ ! -d "$CODE_DIR" ]; then
    echo "Code directory not found: $CODE_DIR" >&2
    exit 3
fi

# Use NUL-separated output to be safe with spaces
while IFS= read -r -d '' file; do
    base="$(basename "$file")"
    dir="$(dirname "$file")"
    name="${base%.*}"
    ext="${base##*.}"

    # compute relative path under CODE_DIR and target output directory
    rel="${file#$CODE_DIR/}"
    rel="${rel#/}"            # strip leading slash if any
    rel_dir="$(dirname "$rel")"
    if [ "$rel_dir" = "." ]; then
        out_dir="$OUT_BASE"
    else
        out_dir="$OUT_BASE/$rel_dir"
    fi
    mkdir -p "$out_dir"

    out="${out_dir}/${name}${ext}.spv"

    echo "Compiling: $file -> $out"
    if glslc "$file" -o "$out"; then
        echo " OK: $out"
    else
        echo " FAIL: $file" >&2
        # continue processing other files instead of exiting to show all failures
    fi
done < <(find "$CODE_DIR" -type f \( "${find_args[@]}" \) -print0)