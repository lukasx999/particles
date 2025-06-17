#!/bin/sh
set -euxo pipefail

clang++ -Wall -Wextra -std=c++23 -pedantic -Og -ggdb main.cc -lraylib -o out
./out
