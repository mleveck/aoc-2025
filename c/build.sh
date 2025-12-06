#!/bin/bash
set -e

# Universal build script for Advent of Code 2025
# Usage:
#   From root:   ./build.sh day02/p1 input.txt
#   From day dir: ../build.sh p1 input.txt
#   Build all:   ./build.sh all

# Find repo root (directory containing this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

# Compiler settings
CC="${CC:-gcc}"
CFLAGS="-std=c99 -Wall -I$REPO_ROOT -g -O3"

# Update compile_commands.json
update_compile_db() {
    local compdb="$REPO_ROOT/compile_commands.json"
    local temp_compdb="$REPO_ROOT/.compile_commands.json.tmp"

    echo "Updating compile_commands.json..."

    # Start JSON array
    echo "[" > "$temp_compdb"

    local first=true

    # Add util.h first so clangd can track symbols in the header
    if [ -f "$REPO_ROOT/util.h" ]; then
        cat >> "$temp_compdb" << EOF
  {
    "directory": "$REPO_ROOT",
    "command": "$CC $CFLAGS -c -x c util.h",
    "file": "util.h"
  }
EOF
        first=false
    fi

    # Find all p1.c and p2.c files in day directories
    for src in "$REPO_ROOT"/day*/p*.c; do
        if [ ! -f "$src" ]; then
            continue
        fi

        local dir="$(dirname "$src")"
        local file="$(basename "$src")"

        # Add comma before all entries except first
        if [ "$first" = true ]; then
            first=false
        else
            echo "," >> "$temp_compdb"
        fi

        # Add JSON entry
        cat >> "$temp_compdb" << EOF
  {
    "directory": "$dir",
    "command": "$CC $CFLAGS -c $file",
    "file": "$file"
  }
EOF
    done

    # Close JSON array
    echo "" >> "$temp_compdb"
    echo "]" >> "$temp_compdb"

    # Atomic move
    mv "$temp_compdb" "$compdb"
    echo "compile_commands.json updated"
}

# Parse arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <part> [args...]"
    echo "  part: 'p1', 'p2', 'day01/p1', or 'all'"
    echo "Examples:"
    echo "  $0 p1 input.txt"
    echo "  $0 day02/p1 sample_input.txt"
    echo "  $0 all              # Build all existing code"
    exit 1
fi

PART_ARG="$1"
shift

# Check for "all" mode
if [ "$PART_ARG" = "all" ]; then
    echo "Building all days..."
    FAILED=()
    SUCCEEDED=()

    for src in "$REPO_ROOT"/day*/p*.c; do
        if [ ! -f "$src" ]; then
            continue
        fi

        day_dir="$(dirname "$src")"
        part="$(basename "$src" .c)"
        day_name="$(basename "$day_dir")"

        echo ""
        echo "Building $day_name/$part..."
        cd "$day_dir"

        if $CC $CFLAGS -o "$part" "${part}.c" 2>&1; then
            echo "  ✓ $day_name/$part"
            SUCCEEDED+=("$day_name/$part")
        else
            echo "  ✗ $day_name/$part failed"
            FAILED+=("$day_name/$part")
        fi
    done

    echo ""
    echo "================================"
    echo "Build Summary:"
    echo "  Succeeded: ${#SUCCEEDED[@]}"
    echo "  Failed: ${#FAILED[@]}"

    if [ ${#FAILED[@]} -gt 0 ]; then
        echo ""
        echo "Failed builds:"
        for f in "${FAILED[@]}"; do
            echo "  - $f"
        done
    fi

    echo ""
    update_compile_db

    exit 0
fi

# Detect if we're in a day directory or need to parse day from argument
if [[ "$PART_ARG" == */* ]]; then
    # Format: day02/p1
    DAY_DIR="$REPO_ROOT/$(dirname "$PART_ARG")"
    PART="$(basename "$PART_ARG")"
else
    # Format: p1 (assume we're in a day directory or current directory)
    if [[ "$(basename "$PWD")" == day* ]]; then
        DAY_DIR="$PWD"
    else
        echo "Error: Must specify day when calling from root (e.g., day02/p1)"
        exit 1
    fi
    PART="$PART_ARG"
fi

# Validate part
if [[ "$PART" != "p1" && "$PART" != "p2" ]]; then
    echo "Error: Part must be 'p1' or 'p2', got: $PART"
    exit 1
fi

# Validate day directory exists
if [ ! -d "$DAY_DIR" ]; then
    echo "Error: Directory not found: $DAY_DIR"
    exit 1
fi

SOURCE_FILE="$DAY_DIR/${PART}.c"
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Error: Source file not found: $SOURCE_FILE"
    exit 1
fi

# Build
echo "Building $(basename "$DAY_DIR")/$PART..."
cd "$DAY_DIR"
$CC $CFLAGS -o "$PART" "${PART}.c"

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful: $DAY_DIR/$PART"

# Update compile_commands.json
update_compile_db

# Run the binary with remaining arguments
if [ $# -gt 0 ]; then
    echo ""
    echo "Running ./$PART $@"
    echo "---"
    "./$PART" "$@"
else
    echo ""
    echo "Tip: Run with: ./$PART [input_file]"
fi
