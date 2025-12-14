#!/bin/bash
# run like: ./run.sh p1 input.txt
part=$1
shift
cd "$(dirname "$0")"
export LIBRARY_PATH=/opt/homebrew/lib
export CPATH=/opt/homebrew/include

# Check if GLPK is available when building p2
if [[ "$part" == "p2" ]]; then
    if ! pkg-config --exists glpk 2>/dev/null && [ ! -f /opt/homebrew/include/glpk.h ]; then
        echo ""
        echo "WARNING: GLPK not found!"
        echo "Day 10 Part 2 requires the GLPK (GNU Linear Programming Kit) library."
        echo ""
        echo "To install on macOS with Homebrew:"
        echo "  brew install glpk"
        echo ""
        echo "Attempting to build anyway..."
        echo ""
    fi
fi

gcc -std=c99  -Wall -I.. -g -o $part $part.c -lglpk && ./$part $@
