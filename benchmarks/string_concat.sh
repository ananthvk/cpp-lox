#!/bin/bash

src=$(bash -c 's=""; for i in {1..128}; do s+="\"ab\"+"; done; s=${s%+}; s+=")==("; for i in {1..128}; do s+="\"axyxyxyxyxyxyxyxyxyxysb\"+"; done; s=${s%+}; s+=")"; echo "$s"')
# Build the program as: g++ -O3 src/*.cpp -Iinclude -lfmt
echo $src > out.lox
echo ";">> out.lox
hyperfine --command-name "cpp-lox" --shell=none --warmup=100 "./a.out -c '$src'"  '/home/shank/clone/craftinginterpreters/a.out out.lox'
