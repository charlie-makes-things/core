#! /bin/bash

#quit if there is an error at any point
set -e 

echo test build script. outputs executables to examples/build/.
echo add -e exe name to run a program after it compiles i.e build_examples.sh -e blob_monster
echo add -s1 to build shaders in examples/build/assets/shaders/
echo add -d1 to make a debug build and run in gdb
echo add -p linux/win/mac to build for that platform
echo

platform=0
debug=""
execname=0
gdb=0

while getopts e:s:p:d: flag
do
    case "${flag}" in
        e) execname=${OPTARG};;
        s) echo compiling shaders...
            ./compile_example_shaders.sh;;
        p) platform=${OPTARG};;
        d) debug="-g"
           gdb=1;;
        :) exit;;
    esac
done

echo $platform build.
if [[ $debug == -g ]]; then
    echo debug build
fi

winlibpath=$PWD/examples/lib/win/lib
winincpath=$PWD/examples/lib/win/include

echo compiling executables
for filename in examples/*.c; do
    if [ -f "$filename" ]; then
       NAME="$(basename $filename .c)"
       
      if [[ $platform == win ]] ; then


            x86_64-w64-mingw32-gcc -std=c99 $debug -D__USE_MINGW_ANSI_STDIO=1 -I$winincpath -L$winlibpath -Wpedantic -Wall -Wextra -Wshadow -Wno-unused-parameter $filename -o examples/build/$NAME.exe -DWIN32_LEAN_AND_MEAN -lmingw32 -mwindows -lm -lSDL3 -lSDL3_mixer -lSDL3_ttf
                       
      elif [[ $platform == linux ]]; then

            gcc -std=c11 $debug -Wpedantic -Wall -Wextra -Wshadow -Wno-unused-parameter $filename -o examples/build/$NAME.out -lm -ldl -lSDL3 -lSDL3_mixer -lSDL3_ttf
            
      elif [[ $platform == mac ]]; then

        echo "mac platfrom doesn't do anything yet. sorry!"

      fi

       
       echo built $filename

    fi
done


if [[ $execname == 0 ]]; then

    echo done.

else

    if [[ $platform == linux ]]; then

        echo running linux exe

        if [[ $debug == "-g" ]]; then
            echo starting debugger
            gdb -ex run ./examples/build/$execname.out
        else
            ./examples/build/$execname.out
        fi

    elif [[ $platform == win ]]; then
        echo running windows exe
        wine examples/build/$execname.exe
    fi

fi


echo running $execname
