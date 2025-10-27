#!/bin/sh
# For development, set compiler to clang
mkdir -p subprojects
meson wrap install fmt
meson wrap install google-benchmark
CXX=clang++ CC=clang meson setup -Db_sanitize=address -Ddevelopment=true -Db_lundef=false -Denable-tests=true -Denable-benchmarks=true --reconfigure builddir
cd builddir
ninja -j8
meson test -v
