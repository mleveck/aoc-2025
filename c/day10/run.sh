#!/bin/bash
# run like: ./run.sh p1 input.txt
# This can only run part 1.  for part 2 you need the glpk library
part=$1
shift
cd "$(dirname "$0")"
gcc -std=c99  -Wall -I.. -g -o $part $part.c && ./$part $@
