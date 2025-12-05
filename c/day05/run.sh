#!/bin/bash
# run like: ./run.sh p1 input.txt
part=$1
shift
cd "$(dirname "$0")"
gcc -std=c99 -Wall -I.. -g -o $part $part.c && ./$part $@
