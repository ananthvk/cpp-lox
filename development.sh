#!/bin/sh
# For development, set compiler to clang
mkdir -p subprojects
meson wrap install gtest
meson wrap install fmt
CXX=clang++ CC=clang meson setup -Db_sanitize=address -Db_lundef=false --reconfigure builddir
cd builddir
meson test