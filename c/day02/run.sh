#!/bin/bash
part=$1
shift
cd "$(dirname "$0")"
gcc -std=c99 -Wall -I.. -g -O3 -o $part $part.c && ./$part $@
