#!/bin/bash
# run like: ./run.sh p1 input.txt
part=$1
shift
cd "$(dirname "$0")"
export LIBRARY_PATH=/opt/homebrew/lib
export CPATH=/opt/homebrew/include
gcc -std=c99  -Wall -I.. -g -o $part $part.c -lglpk && ./$part $@
