#!/bin/bash
part=$1
shift
cd "$(dirname "$0")"
gcc -std=c99 -g -Wall -I.. -o $part $part.c && ./$part $@
