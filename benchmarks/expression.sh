#!/bin/bash
# Build the program as: g++ -O3 src/*.cpp -Iinclude -lfmt
src='(44*63-(-61*-47--49/(51/(93*-21-(((-32*(11*-50-(((52*-39-(48/(-33/(-96*77-76/99-(((91/-75-60/-3-87*83--75/19)*79-59*43-85/-50--25*-92)/90-58/67--26*35-33*-100)/69-95/-29)-39/-17--57*65--3*-90)-42*75--11*-70-92*55)*-78+-41/85-34/-57)*62-11/-87-29/93-34/2)/-15--23/-41-61/-75--92*-74)/-83--82/93--59/68)-21*82--7*7-16*-69)/-13-82*-8+58*73-29*43)/-100-63*50+-28/71-58/-29)/-5--39*37--24*-23)-85*-17-96/-72--3*49)--87*76-33/59)*39-77/-90--44*27)/-91-58*-5-0*-62--50*78'
CLOX_PATH='/home/shank/clone/craftinginterpreters/a.out'
JLOX_PATH='/home/shank/clone/craftinginterpreters/jlox'
PYLOX_PATH='pylox'

echo "print $src" > out.lox
echo ";">> out.lox
hyperfine --command-name "cpp-lox" --shell=none --warmup=10 "./a.out -c '$src'"  "$CLOX_PATH out.lox" "$PYLOX_PATH out.lox" "$JLOX_PATH out.lox"

# Output
# Benchmark 1: cpp-lox
#   Time (mean ± σ):       2.0 ms ±   0.4 ms    [User: 1.0 ms, System: 0.8 ms]
#   Range (min … max):     1.5 ms …   7.6 ms    1505 runs
#  
# Benchmark 2: /home/shank/clone/craftinginterpreters/a.out out.lox
#   Time (mean ± σ):     816.9 µs ± 183.4 µs    [User: 335.5 µs, System: 306.3 µs]
#   Range (min … max):   584.6 µs … 2930.3 µs    3415 runs
#  
# Benchmark 3: pylox out.lox
#   Time (mean ± σ):     277.5 ms ±   7.0 ms    [User: 250.0 ms, System: 24.0 ms]
#   Range (min … max):   266.8 ms … 286.0 ms    10 runs
#  
# Benchmark 4: /home/shank/clone/craftinginterpreters/jlox out.lox
#   Time (mean ± σ):      62.7 ms ±   6.0 ms    [User: 59.1 ms, System: 20.2 ms]
#   Range (min … max):    55.1 ms …  83.3 ms    54 runs
#  
# Summary
#   /home/shank/clone/craftinginterpreters/a.out out.lox ran
#     2.45 ± 0.77 times faster than cpp-lox
#    76.81 ± 18.75 times faster than /home/shank/clone/craftinginterpreters/jlox out.lox
#   339.66 ± 76.71 times faster than pylox out.lox
