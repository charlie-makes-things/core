#! /bin/bash

#quit if there is an error at any poiny
set -e 

echo test build script. outputs executables to examples/build/.
echo add -e exe name to run a program after it compiles i.e build_examples.sh -e blob_monster
echo add -s1 to build shaders in examples/build/assets/shaders/
echo

LD_LIBRARY_PATH=/usr/local/lib

echo compiling executables
for filename in examples/*.c; do
    if [ -f "$filename" ]; then
       NAME="$(basename $filename .c)"
       
       gcc -g -Wpedantic -Wall -Wno-unused-function $filename -o examples/build/$NAME.out -lm -ldl -lSDL3 -lSDL3_mixer -lSDL3_ttf
       
       echo built $filename

    fi
done


while getopts e:s: flag
do
    case "${flag}" in
        e) execname=${OPTARG};;
        s) echo compiling shaders...
            ./compile_example_shaders.sh;;
        :) exit;;
    esac
done

echo
echo running $execname

echo
./examples/build/$execname.out
#eval "$execname"