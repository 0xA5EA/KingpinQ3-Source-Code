##!/bin/sh

export CC=i586-mingw32msvc-gcc
export CCC=i586-mingw32msvc-g++
#export  CCC=/opt/mingw32/bin/i686-w64-mingw32-g++
export SLD=i586-mingw32msvc-ar
#export SLD=/opt/mingw32/bin/i686-w64-mingw32-ar
export LD=i586-mingw32msvc-g++
#export LD=/opt/mingw32/bin/i686-w64-mingw32-g++
export WINDRES=i586-mingw32msvc-windres
#export WINDRES=/opt/mingw32/bin/i686-w64-mingw32-windres
export PLATFORM=mingw32
export ARCH=x86
exec make $*

